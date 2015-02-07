function onPre(v)
    vee.putreg(v, ESP, 32, 40404040)
    
    for i = 0,0x100,4 do
        vee.putmem(v, 40404040-0x80+i, 32, 80808080)
    end

end

function onPost(v)
    ecx = vee.getreg(v, ECX, 32)
    if ecx == nil then
        return false
    end
    
    if ecx == 80808080 and vee.getexit(v) == Return then
	    return true
    end

    return false
end

vee.register(onPre, onPost)
