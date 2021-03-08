// This is an example of how to define a schema for operational monitoring

local moo = import "moo.jsonnet";
local s = moo.oschema.schema("dunedaq.daqmoduletest.consumerinfo");

local info = {
   cl : s.string("class_s", moo.re.ident,
                  doc="A string field"),
   uint8  : s.number("uint8", "u8",
                     doc="An unsigned of 8 bytes"),

   info: s.record("Info", [
       s.field("class_name", self.cl, "consumerinfo", doc="Info class name"),
       s.field("message_size", self.uint8, 0, doc="Message size in bytes"),
       s.field("bytes_received", self.uint8, 0, doc="Total number of bytes received from the queue"),
       s.field("bytes_written", self.uint8, 0, doc="Total number of bytes written to disk")
   ], doc="RandomProducer information")
};

moo.oschema.sort_select(info)