#include "pid.h"

//限幅函数
float limit(float control ,float max){
    if (control>max){
        return max;
    }else if(control<-max){
        return -max;
    }
    return control;
}

//PID计算
float calculate(PID *p,float now){
    float error=p->goal-now;
    p->error_all+=error;
    p->error_all = limit(p->error_all, p->maxErrorAll);
    float error_delta;
    error_delta=error-p->error_last;
    p->error_last=error;
    float control_p,control_i,control_d;
    control_p=limit(p->Kp*error,p->maxp);
    control_i=limit(p->Ki*p->error_all,p->maxi);
    control_d=limit(p->Kd*error_delta,p->maxd);
    float control;
    control=limit(control_p+control_i+control_d,p->max);
    return control;
}

//PID结构体初始化
void PID_Init(PID *p,float kp,float ki,float kd,float goal,float maxp,float maxi,float maxd,float maxErrorAll,float max){
    p->error_all=0;
    p->error_last=0;
    p->goal=goal;
    p->Kp=kp;
    p->Ki=ki;
    p->Kd=kd;
    
    p->maxp=maxp;
    p->maxi=maxi;
    p->maxd=maxd;
    p->max=max;
    p->maxErrorAll=maxErrorAll;
    
}



