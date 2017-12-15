// CDI

// <?xml version='1.0'?>
// <?xml-stylesheet type='text/xsl' href='http://openlcb.org/trunk/prototypes/xml/xslt/cdi.xsl'?>

const char configDefInfo[] PROGMEM = R"(
<cdi xmlns:xsi='http://www.w3.org/2001/XMLSchema-instance' xsi:noNamespaceSchemaLocation='http://openlcb.org/trunk/prototypes/xml/schema/cdi.xsd'>

<identification>
    <manufacturer>OpenLCB</manufacturer>
    <model>OlcbServoPCA9685</model>
    <hardwareVersion>1.0</hardwareVersion>
    <softwareVersion>0.4</softwareVersion>
</identification>

<segment origin='12' space='253'> <!-- bypasses magic, nextEID, nodeID -->
  <group>
    <name>Node ID</name>
    <description>User-provided description of the node</description>
    <string size='20'><name>Node Name</name></string>
    <string size='24'><name>Node Description</name></string>
  </group>
  <group>
    <name>Servos</name>
    <description>Define events associated with servos</description>
    <group replication='8'>
      <name>Servos</name>
      <repname>Servo</repname>
      <string size='16'><name>Description</name></string>
      <eventid><name>Position 1 Event</name></eventid>
      <eventid><name>Position 2 Event</name></eventid>
      <eventid><name>Position 3 Event</name></eventid>
      <int size='2'>Position 1 (800-2200 us)</int>
      <int size='2'>Position 2 (800-2200 us)</int>
      <int size='2'>Position 3 (800-2200 us)</int>
      <int size='1'>Rate decending(0-100)</int>
      <int size='1'>Rate ascending(0-100)</int>
      <int size='1'>Bounce level (0-16)</int>
      <int size='1'></int>
    </group>
  </group>
</segment>
<segment origin='0' space='253'> <!-- stuff magic to trigger resets -->
  <name>Reset</name>
  <description>Controls reloading and clearing node memory. Board must be restarted for this to take effect.</description>
  <int size='4'>
    <map>
      <relation><property>3998572261</property><value>(No reset)</value></relation>
      <relation><property>3998561228</property><value>User clear: New default EventIDs, blank strings</value></relation>
      <relation><property>0</property><value>Mfg clear: Reset all, including Node ID</value></relation>
    </map>
  </int>
</segment>

</cdi>)";



/*  original
const char configDefInfo[] PROGMEM = R"(
<?xml version='1.0'?>
<?xml-stylesheet type='text/xsl' href='http://openlcb.org/trunk/prototypes/xml/xslt/cdi.xsl'?>
<cdi xmlns:xsi='http://www.w3.org/2001/XMLSchema-instance' xsi:noNamespaceSchemaLocation='http://openlcb.org/trunk/prototypes/xml/schema/cdi.xsd'>

<identification>
    <manufacturer>OpenLCB</manufacturer>
    <model>OlcbBasicNode</model>
    <hardwareVersion>1.0</hardwareVersion>
    <softwareVersion>0.4</softwareVersion>
</identification>

<segment origin='0' space='253'>
    <name>Node ID</name>
    <description>User-provided description of the node</description>
    <group offset='94'>
        <string size='20'>
            <name>Node Name</name>
        </string>
        <string size='24'>
            <name>Node Description</name>
        </string>
    </group>
</segment>

<segment origin='10' space='253'>
    <name>I/O Events</name>
    <description>Define events associated with input and output pins</description>
    <group replication='2'>
        <name>Inputs</name>
        <repname>Input</repname>
        <string size='16' offset='150'>
            <name>Description</name>
        </string>
        <eventid offset='-164' ><!-- forward 150, add 16, back 166, then forward 2-->
            <name>Activation Event</name>
        </eventid>
        <eventid offset='2' >
            <name>Inactivation Event</name>
        </eventid>
    </group>
    <group replication='2'>
        <name>Outputs</name>
        <repname>Output</repname>
        <string size='16' offset='150'>
            <name>Description</name>
        </string>
        <eventid offset='-164' ><!-- forward 150, add 16, back 166, then forward 2-->
            <name>Set Event</name>
        </eventid>
        <eventid offset='2' >
            <name>Reset Event</name>
        </eventid>
    </group>
</segment>

<segment origin='0' space='253'>
    <name>Reset</name>
    <description>Controls reloading and clearing node memory. Board must be restarted for this to take effect.</description>
    <int size='4'>
        <map>
            <relation><property>3998572261</property><value>(No reset)</value></relation>
            <relation><property>3998561228</property><value>User clear: New default EventIDs, blank strings</value></relation>
            <relation><property>0</property><value>Mfg clear: Reset all, including Node ID</value></relation>
        </map>
    </int>
</segment>

</cdi>)";
*/
