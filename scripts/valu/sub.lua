function onPre(v)
    --say that we could have a 32-bit value in any general purpose register 
    regClass = {}
    regClass[CLASS] = GenericRegister 
    regClass[VALUE] = 49
    regClass[WIDTH] = 32

    vee.setregclass(v, regClass)

    --and then say that we have some other 32-bit value in any general purpose
    --register
    regClass = {}
    regClass[CLASS] = GenericRegister
    regClass[VALUE] = 10
    regClass[WIDTH] = 32

    vee.setregclass(v, regClass)
end

function checkReg(v, reg, width, val)
    tmp = vee.getreg(v, reg, width)
    if tmp ~= nil then
        if tmp == val then
            return true
        end
    end

    return false
end

function onPost(v)
    --check and see if we branched to the value we wanted to branch to
    eip = vee.getreg(v, EIP, 32)
    if eip == nil then
        return false
    end

    if vee.getexit(v) == Return then
        --if we branched where we wanted to, then check and see if any GPR
        --contains the result of 0x1 + 0x2 
        if  checkReg(v, EAX, 32, 39) or
            checkReg(v, EBX, 32, 39) or
            checkReg(v, ECX, 32, 39) or
            checkReg(v, EDX, 32, 39) or
            checkReg(v, ESI, 32, 39) or
            checkReg(v, EDI, 32, 39) or
            checkReg(v, EBP, 32, 39) then
            return true
        end
    end

    return false
end

vee.register(onPre, onPost)
