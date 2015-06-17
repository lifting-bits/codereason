function onPre(v)
    vee.putreg(v, ECX, 32, 0)
end

function onPost(v)
    eax = vee.getreg(v, EAX, 32)

    if eax == 1 then
        return true
    end

    return false
end
