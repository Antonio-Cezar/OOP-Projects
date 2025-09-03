#include <pcap.h>
#include <iostream>
#include <iomanip>
#include <cstring>
#include <arpa/inet.h>

// Ethernet
struct EthHdr {
    uint8_t dst[6];
    uint8_t src[6];
    uint16_t type; // big-endian
} __attribute__((packed));

// IPv4 (no options)
struct IPv4Hdr {
    uint8_t ver_ihl;      // version(4) + IHL(4)
    uint8_t tos;
    uint16_t len;
    uint16_t id;
    uint16_t flags_frag;
    uint8_t ttl;
    uint8_t proto;
    uint16_t csum;
    uint32_t src;
    uint32_t dst;
} __attribute__((packed));

// TCP (no options)
struct TCPHdr {
    uint16_t sport;
    uint16_t dport;
    uint32_t seq;
    uint32_t ack;
    uint8_t  off_res_ns;  // data offset(4) + flags bits
    uint8_t  flags;
    uint16_t win;
    uint16_t sum;
    uint16_t urgp;
} __attribute__((packed));

// UDP
struct UDPHdr {
    uint16_t sport;
    uint16_t dport;
    uint16_t len;
    uint16_t sum;
} __attribute__((packed));

// ICMP
struct ICMPHdr {
    uint8_t type;
    uint8_t code;
    uint16_t csum;
    uint32_t rest;
} __attribute__((packed));

static void print_mac(const uint8_t m[6]) {
    std::ios::fmtflags f(std::cout.flags());
    std::cout << std::hex << std::setfill('0')
              << std::setw(2) << (int)m[0] << ":"
              << std::setw(2) << (int)m[1] << ":"
              << std::setw(2) << (int)m[2] << ":"
              << std::setw(2) << (int)m[3] << ":"
              << std::setw(2) << (int)m[4] << ":"
              << std::setw(2) << (int)m[5];
    std::cout.flags(f);
}

static const char* ip_to_cstr(uint32_t ip_be, char* buf, size_t sz) {
    in_addr a{ ip_be };
    return inet_ntop(AF_INET, &a, buf, sz);
}

void on_packet(u_char* /*user*/, const pcap_pkthdr* h, const u_char* bytes) {
    if (h->caplen < sizeof(EthHdr)) return;

    auto* eth = reinterpret_cast<const EthHdr*>(bytes);
    uint16_t ethType = ntohs(eth->type);

    std::cout << "---- " << h->len << " bytes ----\n";
    std::cout << "ETH  ";
    print_mac(eth->src);
    std::cout << " -> ";
    print_mac(eth->dst);
    std::cout << "  type=0x" << std::hex << ethType << std::dec << "\n";

    if (ethType != 0x0800) { // not IPv4
        std::cout << "(non-IPv4 frame)\n";
        return;
    }

    const u_char* ip_ptr = bytes + sizeof(EthHdr);
    if (h->caplen < sizeof(EthHdr) + sizeof(IPv4Hdr)) return;

    auto* ip = reinterpret_cast<const IPv4Hdr*>(ip_ptr);
    uint8_t ihl = (ip->ver_ihl & 0x0F) * 4;
    if (ihl < 20) return; // invalid IHL

    char src[INET_ADDRSTRLEN]{}, dst[INET_ADDRSTRLEN]{};
    ip_to_cstr(ip->src, src, sizeof(src));
    ip_to_cstr(ip->dst, dst, sizeof(dst));

    std::cout << "IP   " << src << " -> " << dst
              << "  proto=" << (int)ip->proto
              << "  ttl=" << (int)ip->ttl << "\n";

    const u_char* l4 = ip_ptr + ihl;
    size_t remain = h->caplen - (sizeof(EthHdr) + ihl);

    switch (ip->proto) {
        case 6: { // TCP
            if (remain < sizeof(TCPHdr)) { std::cout << "(truncated TCP)\n"; break; }
            auto* tcp = reinterpret_cast<const TCPHdr*>(l4);
            std::cout << "TCP  " << ntohs(tcp->sport) << " -> " << ntohs(tcp->dport)
                      << "  seq=" << ntohl(tcp->seq) << "\n";
            break;
        }
        case 17: { // UDP
            if (remain < sizeof(UDPHdr)) { std::cout << "(truncated UDP)\n"; break; }
            auto* udp = reinterpret_cast<const UDPHdr*>(l4);
            std::cout << "UDP  " << ntohs(udp->sport) << " -> " << ntohs(udp->dport)
                      << "  len=" << ntohs(udp->len) << "\n";
            break;
        }
        case 1: { // ICMP
            if (remain < sizeof(ICMPHdr)) { std::cout << "(truncated ICMP)\n"; break; }
            auto* icmp = reinterpret_cast<const ICMPHdr*>(l4);
            std::cout << "ICMP type=" << (int)icmp->type
                      << " code=" << (int)icmp->code << "\n";
            break;
        }
        default:
            std::cout << "(other L4 protocol)\n";
            break;
    }
}

int main(int argc, char** argv) {
    const char* dev = nullptr;
    const char* filter = nullptr;

    if (argc >= 2) dev = argv[1];
    if (argc >= 3) filter = argv[2];

    char errbuf[PCAP_ERRBUF_SIZE]{};

    // Choose default device if not provided
    if (!dev) {
        dev = pcap_lookupdev(errbuf);
        if (!dev) {
            std::cerr << "pcap_lookupdev failed: " << errbuf << "\n";
            return 1;
        }
    }
    std::cout << "Using device: " << dev << "\n";

    // Open device
    pcap_t* handle = pcap_open_live(dev, BUFSIZ, 1 /*promisc*/, 1000 /*ms*/, errbuf);
    if (!handle) {
        std::cerr << "pcap_open_live failed: " << errbuf << "\n";
        return 1;
    }

    // Apply optional BPF filter (e.g. "tcp or udp")
    if (filter) {
        bpf_program fp{};
        if (pcap_compile(handle, &fp, filter, 1, PCAP_NETMASK_UNKNOWN) == -1) {
            std::cerr << "pcap_compile failed: " << pcap_geterr(handle) << "\n";
            pcap_close(handle);
            return 1;
        }
        if (pcap_setfilter(handle, &fp) == -1) {
            std::cerr << "pcap_setfilter failed: " << pcap_geterr(handle) << "\n";
            pcap_freecode(&fp);
            pcap_close(handle);
            return 1;
        }
        pcap_freecode(&fp);
        std::cout << "Filter applied: " << filter << "\n";
    }

    std::cout << "Sniffing... (Ctrl+C to stop)\n";
    if (pcap_loop(handle, 0 /*infinite*/, on_packet, nullptr) == -1) {
        std::cerr << "pcap_loop error: " << pcap_geterr(handle) << "\n";
        pcap_close(handle);
        return 1;
    }

    pcap_close(handle);
    return 0;
}
