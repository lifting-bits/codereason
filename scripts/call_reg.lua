function onPre(v)
    vee.putreg(v, EAX, 32, 80808080)
    vee.putreg(v, EBX, 32, 80808080)
    vee.putreg(v, ECX, 32, 80808080)
    vee.putreg(v, EDX, 32, 80808080)
    vee.putreg(v, ESI, 32, 80808080)
    vee.putreg(v, EDI, 32, 80808080)
    vee.putreg(v, EBP, 32, 80808080)
    vee.putreg(v, ESP, 32, 40404040)
end

function onPost(v)
    eip = vee.getreg(v, EIP, 32)
    --ret_addr = vee.getmem(v, vee.getreg(v, ESP, 32), 32)
    
    if eip == nil then
        return false
    end

    if eip == 80808080 then 
        return true
    end

    return false
end

vee.register(onPre, onPost)
