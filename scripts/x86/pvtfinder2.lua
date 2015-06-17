function onPre(v)
    vee.putreg(v, ESP, 32, 40000000)
    vee.putreg(v, EAX, 32, 80808080)

    for i=0,4096 do
        vee.putmem(v, 80808080+i, 8, 20)
    end
end

function onPost(v)
    esp = vee.getreg(v, ESP, 32)
    
    if esp ~= nil and esp > 80808080-16 and esp < 80808080+16 then
        if vee.getexit(v) == Return then
            return true
        end
        return false
    end

    eip = vee.getreg(v, EIP, 32)

    if eip ~= nil and eip == 20202020 then
        return true
    end

    return false
end

vee.register(onPre, onPost)
