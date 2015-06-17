function onPre(v)
    vee.putreg(v, EAX, 32, 0x40000000)
    vee.putmem(v, 0x40000000, 32, 0x20202020)
end

function onPost(v)
    eip = vee.getreg(v, EIP, 32)

    if eip == nil then
        return false
    end

    if eip == 0x20202020 then
        return true
    end

    return false
end

vee.register(onPre, onPost)
