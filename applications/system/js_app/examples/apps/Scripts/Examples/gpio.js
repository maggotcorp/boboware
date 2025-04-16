let gpio = require("gpio");

// initialize pins
<<<<<<< HEAD
gpio.init("PC3", "outputPushPull", "up"); // pin, mode, pull
print("PC3 is initialized as outputPushPull with pull-up");
=======
let led = gpio.get("pc3"); // same as `gpio.get(7)`
let led2 = gpio.get("pa7"); // same as `gpio.get(2)`
let pot = gpio.get("pc0"); // same as `gpio.get(16)`
let button = gpio.get("pc1"); // same as `gpio.get(15)`
led.init({ direction: "out", outMode: "push_pull" });
pot.init({ direction: "in", inMode: "analog" });
button.init({ direction: "in", pull: "up", inMode: "interrupt", edge: "falling" });
>>>>>>> deva

gpio.init("PC1", "input", "down"); // pin, mode, pull
print("PC1 is initialized as input with pull-down");

<<<<<<< HEAD
// let led on PC3 blink
gpio.write("PC3", true); // high
delay(1000);
gpio.write("PC3", false); // low
delay(1000);
gpio.write("PC3", true);  // high
delay(1000);
gpio.write("PC3", false); // low

// read value from PC1 and write it to PC3
while (true) {
    let value = gpio.read("PC1");
    gpio.write("PC3", value);

    value ? print("PC1 is high") : print("PC1 is low");

    delay(100);
}
=======
// cycle led pwm
print("Commencing PWM (PA7)");
eventLoop.subscribe(eventLoop.timer("periodic", 10), function (_, _item, led2, state) {
    led2.pwmWrite(10000, state);
    return [led2, (state + 1) % 101];
}, led2, 0);

// read potentiometer when button is pressed
print("Press the button (PC1)");
eventLoop.subscribe(button.interrupt(), function (_, _item, pot) {
    print("PC0 is at", pot.readAnalog(), "mV");
}, pot);
>>>>>>> deva


// possible pins https://docs.flipper.net/gpio-and-modules#miFsS
// "PA7" aka 2
// "PA6" aka 3
// "PA4" aka 4
// "PB3" aka 5
// "PB2" aka 6
// "PC3" aka 7
// "PA14" aka 10
// "PA13" aka 12
// "PB6" aka 13
// "PB7" aka 14
// "PC1" aka 15
// "PC0" aka 16
// "PB14" aka 17

// possible modes
// "input"
// "outputPushPull"
// "outputOpenDrain"
// "altFunctionPushPull"
// "altFunctionOpenDrain"
// "analog"
// "interruptRise"
// "interruptFall"
// "interruptRiseFall"
// "eventRise"
// "eventFall"
// "eventRiseFall"

// possible pull
// "no"
// "up"
// "down"