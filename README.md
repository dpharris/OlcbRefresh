# OlcbRefresh
This is a refresh of the Arduino base libs, ie OlcbStarLibraries.  

It is meant to simplify and extend the Arduino code.

Changes: 
1. Added support for multiple processors: 
    Arduino, Teensy, Tiva
2. Simplified the writing of CDI/xml for the node
    by making a struct{} that parallels the xml structure.   
3. To speed up eventID processing, 
    uses Index[]'s to allow eventID's to be sorted and searched.  


    