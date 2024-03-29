control d(out bit<32> x) {
    @name("d.cinst.a1") action cinst_a1_0() {
    }
    @name("d.cinst.a2") action cinst_a2_0() {
    }
    @name("d.cinst.t") table cinst_t {
        actions = {
            cinst_a1_0();
            cinst_a2_0();
        }
        default_action = cinst_a1_0();
    }
    apply {
        {
            @name("d.cinst.hasReturned") bool cinst_hasReturned = false;
            switch (cinst_t.apply().action_run) {
                cinst_a1_0: 
                cinst_a2_0: {
                    cinst_hasReturned = true;
                }
                default: {
                    cinst_hasReturned = true;
                }
            }
        }
    }
}

control dproto(out bit<32> x);
package top(dproto _d);
top(d()) main;

