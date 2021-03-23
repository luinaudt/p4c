#include <core.p4>

header ethernet_t {
    bit<48> dst_addr;
    bit<48> src_addr;
    bit<16> eth_type;
}

struct Headers {
    ethernet_t eth_hdr;
}

extern bit<48> simple_extern(out bool VfrH, bool grLo);
parser p(packet_in pkt, out Headers hdr) {
    state start {
        pkt.extract<ethernet_t>(hdr.eth_hdr);
        transition accept;
    }
}

control ingress(inout Headers h) {
    @name("ingress.tmp") bool tmp_0;
    @hidden action gauntlet_extern_arguments_3l29() {
        h.eth_hdr.src_addr = simple_extern(tmp_0, false);
    }
    @hidden table tbl_gauntlet_extern_arguments_3l29 {
        actions = {
            gauntlet_extern_arguments_3l29();
        }
        const default_action = gauntlet_extern_arguments_3l29();
    }
    apply {
        tbl_gauntlet_extern_arguments_3l29.apply();
    }
}

parser Parser(packet_in b, out Headers hdr);
control Ingress(inout Headers hdr);
package top(Parser p, Ingress ig);
top(p(), ingress()) main;

