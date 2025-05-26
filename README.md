3f429b0 - the client sends Hello; the server prints it; works  
67533f4 - sending the int '10'; the client sends it as a string, the server parses the string to int; corresponds to serverside 869cb44  
88be0d2 - client converts an int to string and then sends it; corresponds to serverside 9172980  
f6b33d7 - sending 2 numbers; using int arrays in client and sersver; client creates a string from 2 ints, server parses the 2 numbers back into ints; corresponds to serverside 3312f34  
8d6052a - client sends 400 uints every 2 seconds; server receives well and prints the full body; serverside 881ecd0  
