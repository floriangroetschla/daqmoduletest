local moo = import "moo.jsonnet";
local ns = "dunedaq.daqmoduletest.conf";
local s = moo.oschema.schema(ns);

local types = {
    size: s.number("Size", "u8",
                   doc="A count of very many things"),
    cl : s.string("class_s", moo.re.ident,
                      doc="A string field"),

    conf: s.record("Conf", [
        s.field("message_size", self.size, 4096,
                doc="Size of the messages in bytes"),
        s.field("output_dir", self.cl, "output", doc="Output dir to write data to"),
    ], doc="Configuration"),

};

moo.oschema.sort_select(types, ns)