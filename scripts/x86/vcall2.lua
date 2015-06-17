function onPre(v)
    vee.putreg(v, ECX, 32, 40000000)
    for i=0,3 do
        vee.putmem(v, 40000000 + (i*4), 32, 80808080)
    end
end

function onPost(v)
    eip = vee.getreg(v, EIP, 32)

    if eip == nil then
        return false
    end

    if eip == 80808080 then
        return true
    end

    return false
end

vee.register(onPre, onPost)
