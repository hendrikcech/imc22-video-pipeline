package congestion

import (
	"math"
	"time"

	"github.com/lucas-clemente/quic-go/internal/protocol"
)

type NoOpSendAlgorithm struct {
}

func (n NoOpSendAlgorithm) SetMaxDatagramSize(count protocol.ByteCount) {
}

func (n NoOpSendAlgorithm) TimeUntilSend(bytesInFlight protocol.ByteCount) time.Time {
	return time.Time{}
}

func (n NoOpSendAlgorithm) HasPacingBudget() bool {
	return true
}

func (n NoOpSendAlgorithm) OnPacketSent(sentTime time.Time, bytesInFlight protocol.ByteCount, packetNumber protocol.PacketNumber, bytes protocol.ByteCount, isRetransmittable bool) {
}

func (n NoOpSendAlgorithm) CanSend(bytesInFlight protocol.ByteCount) bool {
	return true
}

func (n NoOpSendAlgorithm) MaybeExitSlowStart() {
}

func (n NoOpSendAlgorithm) OnPacketAcked(number protocol.PacketNumber, ackedBytes protocol.ByteCount, priorInFlight protocol.ByteCount, eventTime time.Time) {
}

func (n NoOpSendAlgorithm) OnPacketLost(number protocol.PacketNumber, lostBytes protocol.ByteCount, priorInFlight protocol.ByteCount) {
}

func (n NoOpSendAlgorithm) OnRetransmissionTimeout(packetsRetransmitted bool) {
}

func (n NoOpSendAlgorithm) InSlowStart() bool {
	return false
}

func (n NoOpSendAlgorithm) InRecovery() bool {
	return false
}

func (n NoOpSendAlgorithm) GetCongestionWindow() protocol.ByteCount {
	return math.MaxInt64
}
