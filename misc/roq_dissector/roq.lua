-- Dissect RTP-UDP packets and SCReAM RFC8888 acknowledgments in Wireshark sent
-- by rtp-over-quic. The RTP packets have a leading byte that must be removed to
-- be spec compliant. The RFC8888 RTCP packets are dissector by another
-- dissector that can be found at
-- https://github.com/hendrikcech/rfc8888-dissector
--
-- Symlink/move this file to its own folder in the Wireshark plugin folder (e.g.
-- to ~/.config/wireshark/plugins/roq_dissector/roq.lua). Also install the
-- rfc888 dissector the the plugin folder.

do
    roq_proto = Proto("roq","ROQ UDP RTP and RTCP packets")

    roq_proto.fields = {}

    function roq_proto.dissector(buffer, pinfo, tree)
        if (buffer:len() == 0) then return end

        if (buffer(0,1):uint() == 0) then
        -- Drop the leading 0 byte and call the RTP dissector
            Dissector.get("rtp"):call(buffer(1,buffer:len()-1):tvb(), pinfo, tree)
        elseif (buffer(0,2):uint() == 0x80cd) then
            Dissector.get("rfc8888"):call(buffer, pinfo, tree)
        else
            -- TODO: implement gcc's twcc
            return 0
        end

    end

    DissectorTable.get("udp.port"):add(6006, roq_proto)
    DissectorTable.get("udp.port"):add(6007, roq_proto)
    DissectorTable.get("udp.port"):add(6008, roq_proto)
end
