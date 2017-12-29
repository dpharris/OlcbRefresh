// CDI
// NUM_OUTPUTS     8
// NUM_INPUTS      8
// NUM_BOD_INPUTS 24
// NUM_SERVOS     16

const char configDefInfo[] PROGMEM = R"(<?xml version='1.0'?>
<?xml-stylesheet type='text/xsl' href='http://openlcb.org/trunk/prototypes/xml/xslt/cdi.xsl'?>
<cdi xmlns:xsi='http://www.w3.org/2001/XMLSchema-instance' xsi:noNamespaceSchemaLocation='http://openlcb.org/trunk/prototypes/xml/schema/cdi.xsd'>

<identification>
    <manufacturer>OpenLCB</manufacturer>
    <model>OlcbIo</model>
    <hardwareVersion>1.0</hardwareVersion>
    <softwareVersion>0.4</softwareVersion>
</identification>

<segment origin='12' space='253'>
    <group>
        <name>User Identification</name>
        <description>Lets the user add his own description</description>
        <string size='20'>
            <name>Node Name</name>
        </string>
        <string size='24'>
            <name>Node Description</name>
        </string>
    </group>
</segment>

<segment origin='56' space='253'>
    <group replication='8'>
        <name>Digital Outputs</name>
        <repname>Output</repname>
        <string size='16'>
            <name>Description</name>
        </string>
        <eventid>
            <name>Set Event</name>
        </eventid>
        <eventid>
            <name>Reset Event</name>
        </eventid>
    </group>
    <group replication='8'>
        <name>Digital Inputs</name>
        <repname>Input</repname>
        <string size='16'>
            <name>Description</name>
        </string>
        <eventid>
            <name>Activation Event</name>
        </eventid>
        <eventid>
            <name>Inactivation Event</name>
        </eventid>
    </group>
    <group replication='24'>
        <name>Block Occupancy Detector Inputs</name>
        <repname>Block</repname>
        <string size='16'>
            <name>Description</name>
        </string>
        <eventid>
            <name>Occupied Event</name>
        </eventid>
        <eventid>
            <name>Empty Event</name>
        </eventid>
    </group>
    <group replication='16'>
        <name>Turnout Servo Control</name>
        <repname>Servo</repname>
        <string size='16'>
            <name>Description</name>
        </string>
        <eventid>
            <name>Select Main Event</name>
        </eventid>
        <eventid>
            <name>Select Diverging Event</name>
        </eventid>
        <int size='1'>
            <min>0</min>
            <max>180</max>
            <default>90</default>
            <name>Main Servo Position</name>
            <description>Position in Degrees (0-180) </description>
        </int>
        <int size='1'>
            <min>0</min>
            <max>180</max>
            <default>90</default>
            <name>Diverging Servo Position</name>
            <description>Position in Degrees (0-180) </description>
        </int>
    </group>
</segment>

<segment origin='0' space='253'>
    <int size='4'>
        <name>Reset</name>
        <description>Controls reloading and clearing node memory. Board must be restarted for this to take effect.</description>
        <map>
            <relation><property>3998572261</property><value>(No reset)</value></relation>
            <relation><property>3998561228</property><value>User clear: New default EventIDs, blank strings</value></relation>
            <relation><property>0</property><value>Mfg clear: Reset all, including Node ID</value></relation>
        </map>
    </int>
</segment>

</cdi>)";

