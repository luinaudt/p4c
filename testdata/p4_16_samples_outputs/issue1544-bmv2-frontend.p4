#include <core.p4>
#define V1MODEL_VERSION 20180101
#include <v1model.p4>

struct metadata {
}

header ethernet_t {
    bit<48> dstAddr;
    bit<48> srcAddr;
    bit<16> etherType;
}

struct headers {
    ethernet_t ethernet;
}

parser ParserImpl(packet_in packet, out headers hdr, inout metadata meta, inout standard_metadata_t standard_metadata) {
    state start {
        packet.extract<ethernet_t>(hdr.ethernet);
        transition accept;
    }
}

control ingress(inout headers hdr, inout metadata meta, inout standard_metadata_t standard_metadata) {
    @name(".my_drop") action my_drop(@name("smeta") inout standard_metadata_t smeta) {
        mark_to_drop(smeta);
    }
    @name("ingress.set_port") action set_port(@name("output_port") bit<9> output_port) {
        standard_metadata.egress_spec = output_port;
    }
    @name("ingress.mac_da") table mac_da_0 {
        key = {
            hdr.ethernet.dstAddr: exact @name("hdr.ethernet.dstAddr") ;
        }
        actions = {
            set_port();
            my_drop(standard_metadata);
        }
        default_action = my_drop(standard_metadata);
    }
    apply {
        mac_da_0.apply();
        {
            @name("ingress.x_0") bit<16> x_0 = hdr.ethernet.srcAddr[15:0];
            @name("ingress.hasReturned") bool hasReturned = false;
            @name("ingress.retval") bit<16> retval;
            if (x_0 > 16w5) {
                hasReturned = true;
                retval = x_0 + 16w65535;
            } else {
                hasReturned = true;
                retval = x_0;
            }
            hdr.ethernet.srcAddr[15:0] = retval;
        }
    }
}

control egress(inout headers hdr, inout metadata meta, inout standard_metadata_t standard_metadata) {
    apply {
    }
}

control DeparserImpl(packet_out packet, in headers hdr) {
    apply {
        packet.emit<ethernet_t>(hdr.ethernet);
    }
}

control verifyChecksum(inout headers hdr, inout metadata meta) {
    apply {
    }
}

control computeChecksum(inout headers hdr, inout metadata meta) {
    apply {
    }
}

V1Switch<headers, metadata>(ParserImpl(), verifyChecksum(), ingress(), egress(), computeChecksum(), DeparserImpl()) main;

