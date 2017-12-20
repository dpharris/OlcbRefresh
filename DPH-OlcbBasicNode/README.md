# OlcbdBasicNode
This is temporarily named DPH-OldBasicNode

This node is designed to demonstrate using Consumer and Producer eventIDs.

It implements two inputs and two outputs, and is designed to have hardware attached.  Each inpit and output pin has a LED and push-button attached, such that driving the pin low will illuminate the LED, and letting the pin float as an input lets the button state, pushed or released, be sensed.  <br>
    [[I will include a diagram]]

### CDI/xml
The CDI/xml describes the variables and eventIDs that are used by a UI-Tool ,so that it can display them in a useful way.  It is made up of a bunch of phrases bracketed by <xxx> and </xxx>.

Since each input pin has two states, we will want two producer-eventIDs, one for each state, for each input.  Also, each output pin also has two states, and so will have two consumer-eventids each.  

To describe this we need to write the following xml:
```
        <group replication='2'>                                     -- two inputs
            <eventid><name>Activation Event</name></eventid>        -- first eventID
            <eventid><name>Inactivation Event</name></eventid>      -- second eventID
        </group>
        <group replication='2'>                                     -- two outputs
            <eventid><name>Set Event</name></eventid>               -- first eventID
            <eventid><name>Reset Event</name></eventid>             -- second eventID
        </group>

```
In addition, additional descriptive text can be added to name of describe the variables.  For each input and output, we also let the use define a description, so that the entries make more sense:
```
        <group replication='2'>
        <name>Inputs</name>
        <repname>Input</repname>
            <string size='16'><name>Description</name></string>
            <eventid><name>Activation Event</name></eventid>
            <eventid><name>Inactivation Event</name></eventid>
        </group>
        <group replication='2'>
            <name>Outputs</name>
            <repname>Output</repname>
        <string size='16'><name>Description</name></string>
            <eventid><name>Set Event</name></eventid>
            <eventid><name>Reset Event</name></eventid>
        </group>
```
