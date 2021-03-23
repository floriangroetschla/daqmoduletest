// This is an example of how to define a schema for operational monitoring

local moo = import "moo.jsonnet";
local s = moo.oschema.schema("dunedaq.daqmoduletest.consumerinfo");

local info = {
   cl : s.string("class_s", moo.re.ident,
                  doc="A string field"),
   uint8  : s.number("uint8", "u8",
                     doc="An unsigned of 8 bytes"),
   double8 : s.number("double8", "f8",
                     doc="A float of 8 bytes"),
   boolean : s.boolean("Boolean", doc="Boolean"),

   info: s.record("Info", [
       s.field("class_name", self.cl, "consumerinfo", doc="Info class name"),
       s.field("timestamp", self.uint8, 0, doc="Timestamp"),
       s.field("completed_measurement", self.boolean, false, doc="Completion status of the measurement"),
       s.field("completed", self.boolean, false, doc="Completion status of writes"),
       s.field("throughput", self.double8, 0, doc="Throughput"),
       s.field("throughput_from_last_measurement", self.double8, 0, doc="Throughput from last measurement"),
       s.field("message_size", self.uint8, 0, doc="Message size in bytes"),
       s.field("bytes_received", self.uint8, 0, doc="Total number of bytes received from the queue"),
       s.field("bytes_written", self.uint8, 0, doc="Total number of bytes written to disk")
   ], doc="RandomProducer information")
};

moo.oschema.sort_select(info)