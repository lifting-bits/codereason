function onPre(v)
    print "onPre"
    vee.getreg(v, 0x20202020, 0x80808080)
end

function onPost(vee)
    print "onPost"
return true
end

vee.register(onPre, onPost)
