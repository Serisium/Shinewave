void build_report(Controller *controller, report_t report) {
    if(CONTROLLER_A(*controller))
        reportBuffer.buttonMask |= (1 << 0);
    else
        reportBuffer.buttonMask &= ~(1 << 0);

    if(CONTROLLER_B(*controller))
        reportBuffer.buttonMask |= (1 << 1);
    else
        reportBuffer.buttonMask &= ~(1 << 1);

    if(CONTROLLER_X(*controller))
        reportBuffer.buttonMask |= (1 << 2);
    else
        reportBuffer.buttonMask &= ~(1 << 2);

    if(CONTROLLER_Y(*controller))
        reportBuffer.buttonMask |= (1 << 3);
    else
        reportBuffer.buttonMask &= ~(1 << 3);

    if(CONTROLLER_START(*controller))
        reportBuffer.buttonMask |= (1 << 7);
    else
        reportBuffer.buttonMask &= ~(1 << 7);

    if(CONTROLLER_D_UP(*controller))
        reportBuffer.buttonMask |= (1 << 6);
    else
        reportBuffer.buttonMask &= ~(1 << 6);

    if(controller->analog_l > 127)
        reportBuffer.buttonMask |= (1 << 4);
    else
        reportBuffer.buttonMask &= ~(1 << 4);

    reportBuffer.x = controller->joy_x;
    reportBuffer.y = -(controller->joy_y);
    reportBuffer.z = controller->analog_l;
    reportBuffer.rx = controller->c_x;
    reportBuffer.ry = controller->c_y;
    reportBuffer.rz = controller->analog_r;
}

