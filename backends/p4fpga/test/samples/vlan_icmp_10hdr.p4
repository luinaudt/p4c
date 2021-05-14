#include <core.p4>
#include <v1model.p4>

typedef bit<48>     MacAddress;
typedef bit<32>     ip4Addr_t;
typedef bit<128>    IPv6Address;
const bit<16> ETHERTYPE_MPLS = 0x8847;

header ethernet_h {
    MacAddress          dst;
    MacAddress          src;
    bit<16>             type;
}
header ipv6_h {
    bit<4>              version;
    bit<8>              tc;
    bit<20>             fl;
    bit<16>             plen;
    bit<8>              nh;
    bit<8>              hl;
    IPv6Address         src;
    IPv6Address         dst;
}

header ipv4_t {
    bit<4>    version;
    bit<4>    ihl;
    bit<8>    diffserv;
    bit<16>   totalLen;
    bit<16>   identification;
    bit<3>    flags;
    bit<13>   fragOffset;
    bit<8>    ttl;
    bit<8>    protocol;
    bit<16>   hdrChecksum;
    ip4Addr_t srcAddr;
    ip4Addr_t dstAddr;
}


header tcp_h {
    bit<16>             sport;
    bit<16>             dport;
    bit<32>             seq;
    bit<32>             ack;
    bit<4>              dataofs;
    bit<4>              reserved;
    bit<8>              flags;
    bit<16>             window;
    bit<16>             chksum;
    bit<16>             urgptr;
}
header udp_h {
        bit<16> srcPort;
        bit<16> dstPort;
        bit<16> hdrLength;
        bit<16> chksum;
}

header icmp_h{ // ICMP and ICMPv6 same header
    bit<8> h_type;
    bit<8> code;
    bit<16> checksum;
}


header vlan_h{
    bit<3> pri;
    bit<1> cfi;
    bit<12> vid;
    bit<16> etherType;
}

header mpls_h{
    bit<20> label;
    bit<3> exp;
    bit<1> bos;
    bit<8> ttl;
}

struct headers {
    ethernet_h          ethernet;
    ipv4_t              ipv4;
    ipv6_h              ipv6;
    vlan_h              vlan1;
    vlan_h              vlan2;
    mpls_h              mpls1;
    mpls_h              mpls2;
    icmp_h              icmp;
    icmp_h              icmp6;
    udp_h               udp;
    tcp_h               tcp;
}

struct metadata {
    /* empty */
}
//@Xilinx_MaxPacketRegion(1518*8)  // in bits
parser MyParser(packet_in pkt,
                out headers hdr,
                inout metadata meta,
                inout standard_metadata_t standard_metadata) {

    state start {
        pkt.extract(hdr.ethernet);
        transition select(hdr.ethernet.type) {
	    0x8100 &&& 0xEFFF : parse_vlan1;
        0x0800            : parse_ipv4;
	    0x86DD            : parse_ipv6;
	    ETHERTYPE_MPLS    : parse_mpls1;
            default : accept;
        }
    }
    state parse_vlan1{
	pkt.extract(hdr.vlan1);
        transition select(hdr.vlan1.etherType) {
	    0x8100         : parse_vlan2;
            0x0800         : parse_ipv4;
	    ETHERTYPE_MPLS : parse_mpls1;
	    0x86DD         : parse_ipv6;
            default : accept;
        }
    }
    state parse_vlan2{
	pkt.extract(hdr.vlan2);
        transition select(hdr.vlan2.etherType) {
            0x0800 : parse_ipv4;
	    0x86DD : parse_ipv6;
	    ETHERTYPE_MPLS : parse_mpls1;
            default : accept;
        }
    }
    
    state parse_mpls1{
	pkt.extract(hdr.mpls1);
	transition select(hdr.mpls1.bos){
	    0 : parse_mpls2;
	    1 : parse_mpls_bos;
	}
    }
    state parse_mpls2{
	pkt.extract(hdr.mpls2);
	transition parse_mpls_bos;
    }
    state parse_mpls_bos{
	transition select(pkt.lookahead<bit<4>>()){
	    4: parse_ipv4;
	    6: parse_ipv6;
	    default: accept;
	}
    }
    state parse_ipv6 {
        pkt.extract(hdr.ipv6);
        transition select(hdr.ipv6.nh) {
	    58      : parse_icmp6;
            0x11    : parse_udp;
            6       : parse_tcp;
            default : accept;
        }
    }
    state parse_ipv4 {
        pkt.extract(hdr.ipv4);
        transition select(hdr.ipv4.protocol) {
	    1       : parse_icmp;
            6       : parse_tcp;
	    0x11    : parse_udp;
            default : accept;
        }
    }
    state parse_icmp{
	pkt.extract(hdr.icmp);
	transition accept;
    }
    state parse_icmp6{
	pkt.extract(hdr.icmp6);
	transition accept;
    }
    state parse_udp {
        pkt.extract(hdr.udp);
        transition accept;
    }
    state parse_tcp {
        pkt.extract(hdr.tcp);
        transition accept;
    }

}

control MyVerifyChecksum(inout headers hdr, inout metadata meta) {   
    apply {  }
}

control MyIngress(inout headers hdr,
                  inout metadata meta,
                  inout standard_metadata_t standard_metadata) {

    apply {
    }
}


control MyEgress(inout headers hdr,
                 inout metadata meta,
                 inout standard_metadata_t standard_metadata) {
    apply {  }
}

control MyComputeChecksum(inout headers  hdr, inout metadata meta) {
     apply {
    }
}
control MyDeparser(packet_out pkt, in headers hdr) {
    apply {	
        pkt.emit(hdr.ethernet);
        pkt.emit(hdr.vlan1);
        pkt.emit(hdr.vlan2);
        pkt.emit(hdr.mpls1);
        pkt.emit(hdr.mpls2);
        pkt.emit(hdr.ipv4);
        pkt.emit(hdr.ipv6);
        pkt.emit(hdr.icmp);
        pkt.emit(hdr.icmp6);
        pkt.emit(hdr.tcp);
        pkt.emit(hdr.udp);
    }
}

V1Switch(
MyParser(),
MyVerifyChecksum(),
MyIngress(),
MyEgress(),
MyComputeChecksum(),
MyDeparser()
) main;