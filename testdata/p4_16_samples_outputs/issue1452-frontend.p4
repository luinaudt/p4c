control c() {
    @name("c.x") bit<32> x_0;
    @name("c.a") action a(@name("arg") inout bit<32> arg) {
        @name("c.hasReturned") bool hasReturned = false;
        arg = 32w1;
        hasReturned = true;
    }
    apply {
        a(x_0);
    }
}

control proto();
package top(proto p);
top(c()) main;

