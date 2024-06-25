#ifndef CHEONDUNG_GOKART_MOTORCTL_H
#define CHEONDUNG_GOKART_MOTORCTL_H


class motorctl {
public:
    motorctl(int dir_pin, int pwm_pin);
    void Move(int dir, int pwm);
private:
    int _dir_pin;
    int _pwm_pin;
};


#endif //CHEONDUNG_GOKART_MOTORCTL_H
