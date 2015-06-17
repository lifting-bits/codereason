function onPre(v)
    vee.putreg(v, R1, 32, 80808080)
    vee.putreg(v, R2, 32, 80808080)
    vee.putreg(v, R3, 32, 80808080)
    vee.putreg(v, R4, 32, 80808080)
    vee.putreg(v, R5, 32, 80808080)
    vee.putreg(v, R6, 32, 80808080)
    vee.putreg(v, R7, 32, 80808080)
    vee.putreg(v, R8, 32, 80808080)
    vee.putreg(v, R9, 32, 80808080)
    vee.putreg(v, R10, 32, 80808080)
end

function onPost(v)
    eip = vee.getreg(v, R15, 32)

    if eip ~= nil and eip > 80808080-16 and eip < 80808080+16 then
        return true 
    end

    return false
end

vee.register(onPre, onPost)
