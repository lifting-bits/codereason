function onPre(v)
    vee.putreg(v, ESP, 32, 40000000)
    vee.putmem(v, 40000000, 32, 80808080)
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
