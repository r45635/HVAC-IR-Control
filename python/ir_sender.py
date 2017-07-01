import pigpio

class IrSender:

    """
    https://www.raspberrypi.org/forums/viewtopic.php?f=37&t=79978#
    """

    def __init__(self, rpi, gpio, carrier_hz):

        """
        Initialises an IR tx on a rpi's gpio with a carrier of
        carrier_hz.

        http://www.hifi-remote.com/infrared/IR-PWM.shtml
        """

        self.rpi = rpi
        self.gpio = gpio
        self.carrier_hz = carrier_hz
        self.micros = 1000000 / carrier_hz
        self.on_mics = self.micros / 2
        self.off_mics = self.micros - self.on_mics
        self.offset = 0

        self.buffer = []
        self.wid = -1

        rpi.set_mode(gpio, pigpio.OUTPUT)

    def clear_code(self):
        """
        clear_code
        """
        self.buffer = []
        if self.wid >= 0:
            self.rpi.wave_delete(self.wid)
            self.wid = -1

    def construct_code(self):
        """
        construct_code
        """
        if len(self.buffer) > 0:
            pulses = self.rpi.wave_add_generic(self.buffer)
            print("waveform TOTAL {} pulses".format(pulses))
            self.wid = self.rpi.wave_create()

    def send_code(self):
        """
        send_code
        """
        if self.wid >= 0:
            self.rpi.wave_send_once(self.wid)
            while self.rpi.wave_tx_busy():
                pass

    def add_to_code(self, mark, space):
        """
        add_to_code
        """

        # is there room for more pulses?

        if (mark*2) + 1 + len(self.buffer) > 680: # 682 is maximum

            pulses = self.rpi.wave_add_generic(self.buffer)
            print("waveform partial {} pulses".format(pulses))
            self.offset = self.rpi.wave_get_micros()

            # continue pulses from offset
            self.buffer = [pigpio.pulse(0, 0, self.offset)]

        # add mark cycles of carrier
        for _ in range(mark):
            self.buffer.append(pigpio.pulse(1<<self.gpio, 0, self.on_mics))
            self.buffer.append(pigpio.pulse(0, 1<<self.gpio, self.off_mics))

        # add space cycles of no carrier
        self.add_space(space)

    def add_space(self, space):
        """
        add_space
        """
        self.buffer.append(pigpio.pulse(0, 0, space * self.micros))
