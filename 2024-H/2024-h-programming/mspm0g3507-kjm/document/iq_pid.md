# IQ PID������ʹ���ֲ�

## Ŀ¼
1. [����](#����)
2. [���ٿ�ʼ](#���ٿ�ʼ)
3. [��������](#��������)
4. [�߼�����](#�߼�����)
5. [ʹ��ʾ��](#ʹ��ʾ��)
6. [��������ָ��](#��������ָ��)
7. [��������](#��������)
8. [API�ο�](#api�ο�)

## ����

����һ������TI IQmath��ĸ�����PID��������֧�ֶ��������㣬������DSP��Ƕ��ʽϵͳ��

### ��Ҫ����
- ֧��λ��ʽ������ʽPID�㷨
- ���ɻ��ַ��롢�����޷�������޷�����
- �ṩ���ֻ��ֿ����Ͳ���
- ֧��������������
- ����float��IQ��������ʽ
- ������ʵʱ����

## ���ٿ�ʼ

### 1. ����ͷ�ļ�
```c
#include "iq_pid.h"
```

### 2. ����PID������
```c
PID_Controller_t pid_controller;
```

### 3. ��ʼ��������
```c
// ��ʼ��Ϊλ��ʽPID
PID_Init(&pid_controller, PID_TYPE_POSITION);

// ���ʼ��Ϊ����ʽPID
PID_Init(&pid_controller, PID_TYPE_INCREMENT);
```

### 4. ����PID����
```c
pid_controller.Kp = _IQ(1.0);    // ����ϵ��
pid_controller.Ki = _IQ(0.1);    // ����ϵ��  
pid_controller.Kd = _IQ(0.01);   // ΢��ϵ��
```

### 5. ִ��PID����
```c
_iq target = _IQ(100.0);      // Ŀ��ֵ
_iq feedback = _IQ(85.0);     // ����ֵ
_iq output = PID_Calculate(target, feedback, &pid_controller);
```

## ��������

### PID��������

#### ʹ��IQ��ʽ
```c
pid_controller.Kp = _IQ(1.5);     // ����ϵ��
pid_controller.Ki = _IQ(0.2);     // ����ϵ��
pid_controller.Kd = _IQ(0.05);    // ΢��ϵ��
```

#### ʹ��float��ʽ����Ҫת����
```c
pid_controller.Kp = FLOAT_TO_IQ(1.5);
pid_controller.Ki = FLOAT_TO_IQ(0.2);
pid_controller.Kd = FLOAT_TO_IQ(0.05);
```

### ����޷�����
```c
pid_controller.output_max = _IQ(1000.0);   // ������
pid_controller.output_min = _IQ(-1000.0);  // ��С���
pid_controller.enable_output_limit = 1;     // ��������޷�
```

### �����޷�����
```c
pid_controller.integral_max = _IQ(500.0);   // �������ֵ
pid_controller.integral_min = _IQ(-500.0);  // ������Сֵ
pid_controller.enable_integral_limit = 1;   // ���û����޷�
```

## �߼�����

### 1. ���ַ���
�����ϴ�ʱ����ͣ�������ã���ֹ���������

```c
// ���û��ַ���
pid_controller.enable_integral_separation = 1;
pid_controller.integral_separation_threshold = _IQ(50.0);  // ������ֵ
```

### 2. ��������
����С������������ϵͳ������

```c
// ��������
pid_controller.enable_deadzone = 1;
pid_controller.deadzone = _IQ(2.0);  // ������С
```

### 3. ���ֿ�����
��ֹ���ֱ��ͣ���߿������ܡ�

#### ����һ�����ּн������Ƽ���
```c
PID_SetAntiWindupMethod(&pid_controller, ANTI_WINDUP_CLAMPING, _IQ(30.0), _IQ(0.1));
```

#### ��������������㷨
```c
PID_SetAntiWindupMethod(&pid_controller, ANTI_WINDUP_BACK_CALC, _IQ(30.0), _IQ(0.2));
```

#### ���������������ַ�
```c
PID_SetAntiWindupMethod(&pid_controller, ANTI_WINDUP_CONDITIONAL, _IQ(40.0), _IQ(0.1));
```

### 4. ���ƫ��
Ϊ�����ӹ̶�ƫ������

```c
pid_controller.output_offset = _IQ(50.0);  // ���ƫ��50
```

## ʹ��ʾ��

### ʾ��1������ٶȿ��ƣ�λ��ʽPID��
```c
#include "iq_pid.h"

PID_Controller_t speed_pid;

void motor_speed_control_init(void) {
    // ��ʼ��λ��ʽPID
    PID_Init(&speed_pid, PID_TYPE_POSITION);
    
    // ����PID����
    speed_pid.Kp = _IQ(0.8);
    speed_pid.Ki = _IQ(0.15);
    speed_pid.Kd = _IQ(0.02);
    
    // ��������޷�
    speed_pid.output_max = _IQ(1000.0);
    speed_pid.output_min = _IQ(-1000.0);
    
    // ���û��ַ���
    speed_pid.enable_integral_separation = 1;
    speed_pid.integral_separation_threshold = _IQ(100.0);
    
    // ���ÿ�����
    PID_SetAntiWindupMethod(&speed_pid, ANTI_WINDUP_CLAMPING, _IQ(50.0), _IQ(0.1));
}

_iq motor_speed_control(_iq target_speed, _iq actual_speed) {
    return PID_Calculate(target_speed, actual_speed, &speed_pid);
}
```

### ʾ��2���¶ȿ��ƣ�����ʽPID��
```c
#include "iq_pid.h"

PID_Controller_t temp_pid;

void temperature_control_init(void) {
    // ��ʼ������ʽPID
    PID_Init(&temp_pid, PID_TYPE_INCREMENT);
    
    // ����PID����
    temp_pid.Kp = _IQ(2.0);
    temp_pid.Ki = _IQ(0.05);
    temp_pid.Kd = _IQ(0.1);
    
    // ��������
    temp_pid.enable_deadzone = 1;
    temp_pid.deadzone = _IQ(0.5);  // 0.5������
    
    // ��������޷�
    temp_pid.output_max = _IQ(100.0);   // �����ȹ���
    temp_pid.output_min = _IQ(0.0);     // ��С���ȹ���
    
    // ���û����޷�
    temp_pid.integral_max = _IQ(200.0);
    temp_pid.integral_min = _IQ(-200.0);
}

_iq temperature_control(float target_temp, float actual_temp) {
    // ʹ��float�ӿ�
    PID_SetTargetFloat(&temp_pid, target_temp);
    PID_SetFeedbackFloat(&temp_pid, actual_temp);
    
    return PID_Calculate(temp_pid.target, temp_pid.feedback, &temp_pid);
}
```

### ʾ��3��λ���ŷ�����
```c
#include "iq_pid.h"

PID_Controller_t position_pid;

void servo_position_init(void) {
    PID_Init(&position_pid, PID_TYPE_POSITION);
    
    // ����λ�ÿ��Ʋ���
    position_pid.Kp = _IQ(5.0);
    position_pid.Ki = _IQ(0.8);
    position_pid.Kd = _IQ(0.15);
    
    // �ϸ���������
    position_pid.output_max = _IQ(500.0);
    position_pid.output_min = _IQ(-500.0);
    
    // С�����Լ��ٶ���
    position_pid.enable_deadzone = 1;
    position_pid.deadzone = _IQ(0.1);
    
    // ʹ�÷�����㿹����
    PID_SetAntiWindupMethod(&position_pid, ANTI_WINDUP_BACK_CALC, _IQ(20.0), _IQ(0.3));
}

_iq servo_position_control(_iq target_pos, _iq actual_pos) {
    return PID_Calculate(target_pos, actual_pos, &position_pid);
}
```

## ��������ָ��

### 1. PID�������Ų���

#### ��һ����ȷ��Kp
```c
// ��С��ʼ������Kp��ֱ��ϵͳ��ʼ��
pid_controller.Kp = _IQ(0.1);  // ��ʼֵ
pid_controller.Ki = _IQ(0.0);  // ��ʱ�ر�
pid_controller.Kd = _IQ(0.0);  // ��ʱ�ر�
```

#### �ڶ���������Ki
```c
// Kpȷ�����𲽼���Ki
pid_controller.Ki = _IQ(0.01);  // ��Сֵ��ʼ
```

#### ������������Kd
```c
// ������Kd�Ը��ƶ�̬��Ӧ
pid_controller.Kd = _IQ(0.001);  // ͨ����KiС
```

### 2. �������Ž���

| ���ƶ��� | Kp��Χ | Ki��Χ | Kd��Χ | ��ע |
|---------|--------|--------|--------|------|
| ����ٶ� | 0.5-3.0 | 0.1-0.5 | 0.01-0.1 | ��ӦҪ��� |
| �¶ȿ��� | 1.0-5.0 | 0.01-0.2 | 0.05-0.2 | ���Դ���Ӧ�� |
| λ�ÿ��� | 2.0-10.0 | 0.5-2.0 | 0.1-0.5 | ����Ҫ��� |
| ѹ������ | 0.8-4.0 | 0.05-0.3 | 0.02-0.15 | �������� |

### 3. �޷���������

```c
// �����޷���ͨ����Ϊ����޷���50%-80%
pid_controller.integral_max = _IQ(������ֵ * 0.7);

// ����޷�������ִ���������趨
pid_controller.output_max = _IQ(ִ����������);

// ��������Ϊ�������ȵ�2-5��
pid_controller.deadzone = _IQ(�������� * 3);
```

## ��������

### Q1: ʲôʱ��ʹ��λ��ʽPID��ʲôʱ��ʹ������ʽPID��

**λ��ʽPID�����ڣ�**
- ��Ҫ��ȷ���Ƶ�Ŀ��ֵ�ĳ���
- ִ�����ܽ��ܾ���λ��ָ��
- ϵͳ�ȶ���Ҫ���

**����ʽPID�����ڣ�**
- ִ����ֻ�ܽ�������ָ��
- ϵͳ���ڻ��ֱ�������
- ��Ҫ�ֶ�/�Զ��л��ĳ���

### Q2: ���ѡ����ֿ����ͷ�����

- **���ּн���**����ã��ʺϴ��������
- **������㷨**���ʺ϶���Ӧ�ٶ�Ҫ��ߵ�ϵͳ
- **�������ַ�**���ʺ������ϴ��ϵͳ

### Q3: ϵͳ����ô�죿

```c
// ��СKp
pid_controller.Kp = _IQmpy(pid_controller.Kp, _IQ(0.8));

// ����Kd�����û���������⣩
pid_controller.Kd = _IQmpy(pid_controller.Kd, _IQ(1.2));

// ��������
pid_controller.enable_deadzone = 1;
pid_controller.deadzone = _IQ(�ʵ�ֵ);
```

### Q4: ��̬���̫����ô�죿

```c
// ����Ki
pid_controller.Ki = _IQmpy(pid_controller.Ki, _IQ(1.5));

// �������޷��Ƿ��С
pid_controller.integral_max = _IQ(�����ֵ);

// ȷ�����ַ�����ֵ����
pid_controller.integral_separation_threshold = _IQ(���ʵ�ֵ);
```

### Q5: ��δ������������

```c
// ����ʱ����PID
PID_Reset(&pid_controller);

// ʹ�����ƫ��
pid_controller.output_offset = _IQ(Ԥ��ֵ);

// ���û��ַ���
pid_controller.enable_integral_separation = 1;
pid_controller.integral_separation_threshold = _IQ(��Сֵ);
```

## API�ο�

### ���ĺ���

#### PID_Init
```c
void PID_Init(PID_Controller_t *pid, PID_Type_e type);
```
- **����**����ʼ��PID������
- **����**��
  - `pid`: PID������ָ��
  - `type`: PID���ͣ�λ��ʽ/����ʽ��

#### PID_Calculate
```c
_iq PID_Calculate(_iq target, _iq feedback, PID_Controller_t *pid);
```
- **����**��ִ��PID����
- **����**��
  - `target`: Ŀ��ֵ
  - `feedback`: ����ֵ
  - `pid`: PID������ָ��
- **����ֵ**��PID���

#### PID_Reset
```c
void PID_Reset(PID_Controller_t *pid);
```
- **����**������PID������״̬
- **����**��`pid`: PID������ָ��

### ��������

#### Floatת������
```c
float PID_GetOutputFloat(PID_Controller_t *pid);          // ��ȡfloat��ʽ���
float PID_GetErrorFloat(PID_Controller_t *pid);           // ��ȡfloat��ʽ���
void PID_SetTargetFloat(PID_Controller_t *pid, float target);    // ����float��ʽĿ��
void PID_SetFeedbackFloat(PID_Controller_t *pid, float feedback); // ����float��ʽ����
```

#### �����ͺ���
```c
void PID_SetAntiWindupMethod(PID_Controller_t *pid, AntiWindup_Method_e method, 
                            _iq threshold, _iq gain);
```
- **����**�����û��ֿ����ͷ���
- **����**��
  - `method`: �����ͷ���
  - `threshold`: ��������ֵ
  - `gain`: ����������

### ��������ת����
```c
#define IQ_TO_FLOAT(iq_val)    _IQtoF(iq_val)     // IQתfloat
#define FLOAT_TO_IQ(float_val) _IQ(float_val)     // floatתIQ
```

### ������״̬��ѯ
```c
// ֱ�ӷ��ʽṹ���Ա
float current_error = IQ_TO_FLOAT(pid_controller.error);
float current_output = IQ_TO_FLOAT(pid_controller.output);
float integral_value = IQ_TO_FLOAT(pid_controller.integral);
```

---

## �汾��Ϣ
- **�汾**��v2.0
- **��������**��2025-06-21
- **������**��TI IQmath�⣬C99��׼
- **����**��PID�����������Ŷ�

����������ο�Դ��ע�ͻ���ϵ����֧�֡�