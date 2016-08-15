struct Version {
    bit<8> major;
    bit<8> minor;
}

const Version P4_VERSION = { 8w1, 8w2 };
error {
    NoError,
    PacketTooShort,
    NoMatch,
    EmptyStack,
    FullStack,
    OverwritingHeader,
    HeaderTooShort
}

extern packet_in {
    void extract<T>(out T hdr);
    void extract<T>(out T variableSizeHeader, in bit<32> variableFieldSizeInBits);
    T lookahead<T>();
    void advance(in bit<32> sizeInBits);
    bit<32> length();
}

extern packet_out {
    void emit<T>(in T hdr);
}

extern void verify(in bool check, in error toSignal);
action NoAction() {
}
match_kind {
    exact,
    ternary,
    lpm
}

header Header {
    bit<32> data;
}

parser p1(packet_in p, out Header h) {
    state start {
        transition next;
    }
    state next {
        p.extract<Header>(h);
        transition accept;
    }
    state unreachable1 {
        transition unreachable2;
    }
    state unreachable2 {
        transition unreachable1;
    }
}

parser proto(packet_in p, out Header h);
package top(proto _p);
top(p1()) main;
