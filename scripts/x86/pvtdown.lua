function onPre(v)
    vee.putreg(v, ESP, 32, 0x40404040)
end

function onPost(v)
    esp = vee.getreg(v, ESP, 32)

    if esp == nil then
        return false
    end

    if ((esp & 0xffff0000) == 0x40400000) and (esp > 0x40404040+16) then
        if vee.hascalls(v) == false and vee.getexit(v) == Return then
            return true
        end
        return false
    end

    return false
end

vee.register(onPre, onPost)
