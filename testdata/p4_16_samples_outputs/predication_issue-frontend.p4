#include <core.p4>
#define V1MODEL_VERSION 20180101
#include <v1model.p4>

header ethernet_t {
    bit<48> dst_addr;
    bit<48> src_addr;
    bit<16> eth_type;
}

struct Headers {
    ethernet_t eth_hdr;
}

struct Meta {
}

parser p(packet_in pkt, out Headers hdr, inout Meta m, inout standard_metadata_t sm) {
    state start {
        pkt.extract<ethernet_t>(hdr.eth_hdr);
        transition accept;
    }
}

control ingress(inout Headers h, inout Meta m, inout standard_metadata_t sm) {
    @name(".assign_addrs") action assign_addrs(@name("h") inout Headers h_1, @name("check_1") bool check_1, @name("check_2") bool check_2) {
        h_1.eth_hdr.dst_addr = 48w4;
        if (check_1) {
            h_1.eth_hdr.dst_addr = 48w2;
            if (check_2) {
                h_1.eth_hdr.src_addr = 48w3;
            } else {
                h_1.eth_hdr.dst_addr = 48w1;
            }
        }
    }
    apply {
        assign_addrs(h, true, true);
    }
}

control vrfy(inout Headers h, inout Meta m) {
    apply {
    }
}

control update(inout Headers h, inout Meta m) {
    apply {
    }
}

control egress(inout Headers h, inout Meta m, inout standard_metadata_t sm) {
    apply {
    }
}

control deparser(packet_out b, in Headers h) {
    apply {
        b.emit<Headers>(h);
    }
}

V1Switch<Headers, Meta>(p(), vrfy(), ingress(), egress(), update(), deparser()) main;

