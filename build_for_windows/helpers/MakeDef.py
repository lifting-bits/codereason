f = file("a.tmp", "r")
l = f.readline()
k = 1
o = file("VEX.def", "w")
o.write("LIBRARY VEX\n")
o.write("\n")
o.write("EXPORTS\n")
while len(l) > 0:
    j = l.find("\n")
    s = l[:j]
    u = "\t%s @%i\n" % (s, k)
    #print u
    k = k + 1
    o.write(u)
    l = f.readline()
