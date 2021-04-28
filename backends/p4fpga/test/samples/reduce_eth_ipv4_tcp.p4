
#include <core.p4>
#include <v1model.p4>

typedef bit<48>     MacAddress;
typedef bit<32>     ip4Addr_t;
typedef bit<128>    IPv6Address;


header ethernet_h {
    MacAddress          dst;
    MacAddress          src;
    bit<16>             type;
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

struct headers {
    ethernet_h          ethernet;
    ipv4_t              ipv4;
    tcp_h               tcp;
}

struct metadata {
    /* empty */
}
parser MyParser(packet_in pkt,
                out headers hdr,
                inout metadata meta,
                inout standard_metadata_t standard_metadata) {

    state start {
        pkt.extract(hdr.ethernet);
        transition select(hdr.ethernet.type) {
            0x800  : parse_ipv4;
            default : accept;
        }
    }

    state parse_ipv4 {
        pkt.extract(hdr.ipv4);
        transition select(hdr.ipv4.protocol) {
            6       : parse_tcp;
           default : accept;
        }
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
	if(hdr.ethernet.isValid()){
            pkt.emit(hdr.ethernet);
	    if(hdr.ipv4.isValid(){
	        pkt.emit(hdr.ipv4);
		if(hdr.tcp.isValid()){
		    pkt.emit(hdr.tcp);
		}
	    }
	}
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