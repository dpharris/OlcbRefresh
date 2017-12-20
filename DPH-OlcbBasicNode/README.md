# OlcbBasicNode
*This example is temporarily named DPH-OldBasicNode.*

This node is designed to demonstrate using Consumer and Producer eventIDs.

It implements two inputs and two outputs, and is designed to have hardware attached.  

Each inpit and output pin has a LED and push-button attached to it, such that driving the pin low will illuminate the LED, and letting the pin float as an input allows the button state, pushed or released, be sensed.  <br>
    [[I will include a diagram]]

### CDI/xml
The CDI/xml describes the variables and eventIDs that are used by a UI-Tool, so that it can display them in a useful way.  See: (XML Wikipedia)[https://en.wikipedia.org/wiki/XML]

Since each input pin has two states, we will want two producer-eventIDs, one for each state, for each input.  Also, each output pin also has two states, and so will have two consumer-eventids each.  

To describe this we need to write the following xml:
```
        <group replication='2'>                       -- two inputs
            <eventid></eventid>                       -- first eventID
            <eventid></eventid>                       -- second eventID
        </group>
        <group replication='2'>                       -- two outputs
            <eventid></eventid>                       -- first eventID
            <eventid></eventid>                       -- second eventID
        </group>

```
This is pretty basic and is not descriptive.  Therefore, additional descriptive text can be added to name and describe the variables. In addition, for each input and output, we can add a node variable that contains a description of the input/output.  This looks like:
```
        <group replication='2'>                                     -- two inputs
        <name>Inputs</name>                                         -- Group name
        <repname>Input</repname>                                    -- Name labelling each input, with a # added
        <string size='16'><name>Description</name></string>         -- Node-variable description
            <eventid><name>Activation Event</name></eventid>        -- Named eventID
            <eventid><name>Inactivation Event</name></eventid>      -- Named eventID
        </group>
        <group replication='2'>
        <name>Outputs</name>
        <repname>Output</repname>
        <string size='16'><name>Description</name></string>
            <eventid><name>Set Event</name></eventid>
            <eventid><name>Reset Event</name></eventid>
        </group>
```
