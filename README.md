# VACKeyRetrieval
Retrieves VAC module ice encryption key by reversing the LCG seed that it was generated with
<br>
VAC generates the output Ice encryption key using the LCG algorithm, it then sends some generated values.
<br>
We can get the initial LCG key and thus the Ice key from those values because the LCG algorithm is weak. (probably what they do server-side)
