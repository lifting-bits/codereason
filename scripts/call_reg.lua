function onPre(v)
    vee.putreg(v, ESI, 32, 80808080)
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
