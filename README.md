# Qt 미니프로젝트

## 목차

- [주요 기능](#주요-기능)
- [하드웨어 구성(파트리스트/핀맵/회로도)](#하드웨어-구성-파트리스트-핀-맵-회로도)
- [세부 구현 사항](#세부-구현-사항)
- [파일 목록](#파일-목록)
- [UI 구현 예상도](#ui-구현-예상도)
- [시나리오(입차/만차/출차)](#시나리오-0-시스템-시작-시-초기-동기화)
- [하드웨어 소프트웨어 역할 분담](#하드웨어-및-소프트웨어-역할-분담)
- [UART 통신 프로토콜](#uart-통신-프로토콜)
- [소프트웨어 상세 설계](#소프트웨어-상세-설계)

 
# 주요 기능

| **구분** | **담당 영역** | **주요 역할** |
| --- | --- | --- |
| **STM32 (Firmware)** | 물리 제어 및 센싱 | • 초음파 센서 거리 측정 및 차량 접근/통과 감지
|     |     | • 주차 공간 슬롯 점유 관리 (1~8번 비트/배열)
|     |     | • 최하층 주차면 자동 배정 알고리즘 수행
|     |     | • 서보모터(차단기) 개폐 및 자동 복귀 제어
|     |     | • 층별 점유 LED 8개 점등/소등 제어 |
| **Qt (GUI & Data)** | UI 및 데이터/비즈니스 로직 | • 사용자 입력 처리 (차량번호 4자리 입력, 입차/출차/정산 요청)
|     |     | • 입·출차 상태 및 만차 여부 화면 표시
|     |     | • 차량 정보 관리 (차량번호, 배정 층, 입차 시각, 정산 상태)
|     |     | • 주차 시간 및 주차 요금 계산 로직 처리 |


<br>
<br>
<br>

# 하드웨어 구성 (파트리스트 /핀 맵/ 회로도)

## 1. 사용 부품

| No. | 부품명 | 부품번호 | 개수 |
| --- | --- | --- | --- |
| 1 | STM32 M4 | NUCLEO-F411RE | 1 |
| 2 | 서보 모터 | SG90 | 1 |
| 3 | 초음파 센서 | HC-SR04 | 1 |
| 4 | LED |  | 8 |
| 5 | 330Ω 저항 |  | 8 |

## 2. 핀 맵

| **구분** | **부품 / 기능** | **STM32 핀 번호** | **설정 / 페리페럴** | **역할 및 비고** |
| --- | --- | --- | --- | --- |
| **서보모터** | PWM | **PA0** | TIM2_CH1 | 차단기(차단봉) 제어 (50Hz PWM) |
| **초음파센서** | Trig | **PB4** | GPIO Output | 10µs 거리 측정 트리거 신호 |
|  | Echo | **PB6** | GPIO Input | 신호 반사 수신 (**5V Tolerant**) |
| **LED 8개** | LED 1~8 | **PC0 ~ PC7** | GPIO Output | 8개 주차 공간 점유 상태 표시 |


<br>
<br>
<br>

## 3. 하드웨어 회로도
[PDF 문서 열기](./image/qt_design.pdf)

<br>
<br>
<br>

# 세부 구현 사항

## STM32 M4

- 시스템은 1층부터 8층까지 총 8개의 주차 공간을 관리한다.
- 각 층의 주차 공간 점유 상태를 LED 8개를 이용하여 표시한다.
- 차량이 설정 거리 이내로 접근하면 초음파 센서를 이용하여 차량의 접근을 감지한다.
- 차량 접근이 감지되면 STM32는 현재 빈 주차 공간의 존재 여부를 확인한다.
- STM32로부터 빈 주차 공간이 존재함을 확인한 경우 Qt는 차량번호 입력 및 입차 요청 기능을 활성화한다.
- 만차 상태에서는 차단기를 닫힌 상태로 유지한다.
- 입차 요청을 받으면 STM32는 빈 주차 공간 중 가장 낮은 층을 자동으로 배정한다.
- 주차 공간 배정이 완료되면 서보 모터를 동작 시켜 차단기를 개방한다.
- 차단기는 개방 후 일정 시간이 지나면 자동으로 닫히면 배정된 층의 LED를 점등한다.

## QT

- 사용자는 입차 시 Qt UI에 차량번호 뒤 4자리를 입력하고 입차를 요청한다.
- Qt는 입차가 완료된 차량의 차량번호, 배정된 주차 층 및 입차 시간을 저장한다.
- 출차 시 사용자는 Qt UI에 차량번호 뒤 4자리를 입력하고 출차를 요청한다.
- 시스템은 입력된 차량번호를 검색하여 해당 차량의 주차 층 및 입차 정보를 확인한다.
- Qt는 저장된 입차 시간과 현재 시간을 이용하여 총 주차 시간을 계산한다.
- Qt는 총 주차 시간을 기반으로 주차 요금을 계산하여 사용자에게 표시한다.
- 사용자가 정산 버튼을 누르면 해당 차량의 정산을 완료 상태로 처리한다.
- 정산이 완료되면 서보 모터를 동작시켜 차단기를 개방한다.
- 출차가 완료되면 해당 차량이 주차되어 있던 층의 LED를 소등한다.
- STM32로부터 만차 상태를 확인한 경우 Qt는 만차 상태를 표시하고 차량번호 입력 및 입차 요청 기능을 비활성화한다.
- 출차가 완료되면 Qt는 해당 차량의 주차 정보를 삭제 또는 출차 완료 상태로 처리하고,  STM32는 해당 주차 슬롯을 비어있음 상태로 변경한다.
- STM32와 Qt는 UART Serial 통신을 이용하여 주차 상태와 입·출차 관련 명령 및 처리 결과를 교환한다.


# 파일 목록

```c
QT
Parking_UI
├── Parking_UI.pro            # [1] Qt 프로젝트 설정 파일 (serialport 모듈 추가)
├── main.cpp                  # [2] 앱 실행 시작점 (Main 함수)
├── mainwindow.ui             # [3] UI 디자인 파일 (버튼, 테이블, 로그창 배치)
├── mainwindow.h              # [4] UI 이벤트 처리 및 대시보드 헤더
├── mainwindow.cpp            # [5] UI 구현, 요금 계산
├── serialworker.h            # [6] QThread 전용 시리얼 통신 클래스 헤더
└── serialworker.cpp          # [7] 시리얼 포트 데이터 비동기 송수신 구현
```

```c
STM32 M4
hardware
├── main.h                 # 핀 정의(GPIO Pin) 및 전역 헤더
├── parking_hw.h           # [선택] 초음파/서보/LED 제어 함수 선언
├── main.c                 # [1] 메인 루프, 초기화, 센서 감지 및 처리
├── exception.c            # [2] UART 수신 및 EXTI 비상정지 인터럽트 핸들러
├── servo_motor.c          # [3] 차단기 서보모터 구현
├── ultrasonic.c           # [4] 초음파 거리계산 구현
└── led.c                  # [5] 8개 LED Control   
```

# UI 구현 예상도
![FSM](./image/UI_ex.jpg)


# 하드웨어 및 소프트웨어 역할 분담
```c
┌────────────────────────────────────────┐       UART Serial       ┌────────────────────────────────────────┐
│             STM32 (MCU)                │ ◄─────────────────────► │                Qt (PC)                 │
│         [하드웨어 & 슬롯 제어]            │      (115200 bps)       │         [비즈니스 로직 & 관제]            │
├────────────────────────────────────────┤                         ├────────────────────────────────────────┤
│ • 초음파 센서 차량 감지 (거리 측정)        │                         │ • 차량번호 입력 및 유효성 검증             │
│ • 8개 주차 슬롯 EMPTY/OCCUPIED 관리      │                         │ • 차량번호 ↔ 배정 층 매핑 DB 저장          │
│ • 최저 빈 슬롯 자동 탐색 및 배정           │                         │ • 입차/출차 시간 기록                    │
│ • 만차(Full) 상태 자체 판단               │                         │ • 총 주차 시간 및 요금 자동 계산          │
│ • 서보모터 차단기 자동 개폐 제어           │                          │ • 사용자 정산 처리 및 입출차 이력 관리      │
│ • 8개 LED 점등/소등 (물리 상태 동기화)     │                          │ • 주차장 UI 뷰 렌더링                    │
└────────────────────────────────────────┘                         └────────────────────────────────────────┘
```


# UART 통신 프로토콜
> 종단 문자(EOL): `\r\n` (CR+LF)
문자 인코딩: ASCII
데이터/패리티/정지비트: `8-N-1`
통신 속도: `115200 bps`
> 

### Qt → STM32 (명령 전송)

| 명령어 | 매개변수 | 설명 | 예시 |
| --- | --- | --- | --- |
| `?` | 없음 | 현재 8개 슬롯 점유 상태(8bit) 조회 | `?\r\n` |
| `I:<car4>` | `<car4>` (차량번호 뒤 4자리) | 입차 요청 (STM32가 최저 빈 슬롯 배정 및 차단기 개방) | `I:1234\r\n` |
| `O:<floor>` | `<floor>` (1~8) | 정산 완료/출차 요청 (해당 층 비움 처리 및 차단기 개방) | `O:3\r\n` |

### STM32 → Qt (이벤트 및 응답 전송)



| 명령어 | 매개변수 | 설명 | 예시 |
| --- | --- | --- | --- |
| `R` | 없음 | 부팅 완료 + 슬롯 상태 로드 완료 + UART 준비 완료(Ready) | `R\r\n` |
| `D` | 없음 | 입구 초음파 센서에 차량 접근 감지됨 | `D\r\n` |
| `S:<8bit>` | `<8bit>` (8자리 2진수) | 1~8층 점유 비트마스크 (`1`: 점유, `0`: 비어있음, Bit0=1층) | `S:00000101\r\n` |
| `P:<floor>` | `<floor>` (1~8) | 입차 성공: 배정된 최저 층 번호 응답 (LED 점등, 차단기 개방 포함) | `P:1\r\n` |
| `E` | 없음 | 출차 처리 완료: 해당 층 비움 처리 완료 (LED 소등, 차단기 개방 포함) | `E\r\n` |
| `E:<code>` | `<code>` | 잘못된 요청/상태 등의 에러 코드 | `E:2\r\n` |

## 프로토콜 명령어 총괄표

## 상세 패킷 규격 정의

<aside>

### 1.  `S:<8bit>`(STM32 → Qt)

- **데이터 포맷:** 8자리 2진 문자열 (`Bit7 Bit6 Bit5 Bit4 Bit3 Bit2 Bit1 Bit0` = 8층 ~ 1층)
    - 예: `S:00000011` → 1층, 2층 주차 중 / 3~8층 빈자리.
- **발생 조건:** Qt 프로그램 초기 실행 시점 또는 상태 동기화가 필요할 때.
</aside>

<aside>

### 2. `I` (Qt → STM32) & `P:<floor>` (STM32 → Qt)

- **만차 상태 시:** 차량 접근 시 만차 정보 표시
- **Qt 처리:** Qt UI의 차량번호 입력창과 [입차] 버튼을 활성화(Enable)하고 안내 메시지를 띄움.
- **발생 조건:** 차량이 초음파 센서 설정 거리(예: 10cm 이내)로 접근했을 때 STM32가 1회 발행.
</aside>

<aside>

### 3. `I:<car4>` (Qt → STM32) & `P:<floor>` (STM32 → Qt)

- **동작 흐름:**
    1. Qt가 `I:<car4>` 송신.
    2. STM32는 1~8번 슬롯 중 가장 낮은 빈 슬롯(예: 2층)을 탐색.
        - **빈 슬롯 존재 시:** 해당 슬롯을 `OCCUPIED`로 변경 → 해당 LED 점등 → 차단기 개방 → `P:2` 송신.
        - **만차 시:** `F` 송신.
    3. Qt는 `P:2`를 받으면 입력된 차량번호와 2층 매핑 정보 및 현재 입차 시간을 DB에 기록.
- **발생 조건:** 사용자가 Qt에 차량번호 4자리를 입력하고 [입차] 버튼을 눌렀을 때.
</aside>

<aside>

### 4. `O:<floor>` (Qt→ STM32) & `E:<floor>` (STM32 → Qt)

- **동작 흐름:**
    1. Qt가 자체 DB에서 해당 차량이 주차된 층(예: 3층)을 찾아 `O:3` 송신.
    2. STM32는 3번 슬롯을 `EMPTY`로 변경 → 3층 LED 소등 → 차단기 개방 → `E:3` 송신.
    3. Qt는 `E:3`를 수신한 후 DB에서 해당 차량의 주차 상태를 출차 완료로 갱신.
- **발생 조건:** 사용자가 차량번호를 조회해 요금을 정산하고 [출차/정산 완료]를 눌렀을 때.
- **에러 코드 체계**
    - `E:1` : 잘못된 파라미터 / 유효 범위 초과 (예: 1~8층 외 번호)
    - `E:2` : 상태 에러 (예: 이미 비어있는 층에 대한 출차 시도)
    - `E:3` : 만차 에러 (빈 슬롯 없음)
    - `E:0` : 정의되지 않은 알 수 없는 명령 수신
</aside>

### 시나리오 0. 시스템 시작 시 초기 동기화

```
[Qt 관제 UI]                      [STM32]
     │                              │
     ├─ 프로그램 시작 시 상태 질의 ──>│
     ├──────── ? ──────────────────>│
     │                              ├─ 현재 8개 핀 상태 읽기
     │<─────── S:00000011 ──────────┤ (1층, 2층 점유 중)
     ├─ 1, 2층 레드 / 3~8층 그린 갱신 │
```

### 시나리오 1. 정상 입차 프로세스 (1층 배정)

```c
[하드웨어/차량]            [STM32]                           [Qt 관제 UI]
      │                       │                                   │
      ├─ 초음파 센서 감지 ───>│                                     │
      │                       ├────────────────── D ─────────────>│
      │                       │                                   ├─ 차량번호 입력창 활성화
      │                       │                                    ├─ "1234" 입력 후 [입차] 클릭
      │                       │<────────────── I:1234 ─────────────┤
      │                       ├─ 최저 빈 층(1층) 탐색 & 점유 설정    │
      │                       ├─ 1층 LED(PC0) ON                   │
      │                       ├─ 서보모터 차단기 개방                │
      │                       ├────────────── P:1 ────────────────>│
      │                       │                                    ├─ DB 저장: [1234 - 1층 - 14:00:00]
      │                       │<delay 3s>                          │
      │                       ├─ 서보모터 차단기 자동 닫힘           │
```

### 시나리오 2. 만차 시 입차 요청 프로세스

```
[하드웨어/차량]            [STM32]                   [Qt 관제 UI]
      │                       │                           │
      ├─ 초음파 센서 감지 ───>│                             │
      │                       ├──────────────── D ───────>│
      │                       │<──────── F ───────────────┤
      │                       ├                           │
      │                       ├─ (차단기 닫힘 유지)         │
      │                       ├                            │
      │                       │                            ├─ "현재 만차입니다" 팝업 표시
```

### 시나리오 3. 정산 및 출차 프로세스 (1층 1234 차량 출차)

```
[사용자]                    [Qt 관제 UI]                           [STM32]
   │                             │                                    │
   ├─ "1234" 차량번호 조회 ─────>│                                      │
   │                             ├─ 1층 주차 확인 / 주차시간 계산        │
   │                             ├─ 요금 2,000원 화면 표시              │
   ├─ [정산 완료] 버튼 클릭 ────>│                                      │
   │                             ├────────── O:1 ───────────────────>│
   │                             │                                    ├─ 1층 슬롯 비움(EMPTY)
   │                             │                                    ├─ 1층 LED(PC0) OFF
   │                             │                                    ├─ 서보모터 차단기 개방
   │                             │<────────── E:1 ──────────────────┤
   │                             ├─ DB 1234 주차 이력 완료 처리       │
   │                             │                                    │<delay 3s>
   │                             │                                    ├─ 서보모터 차단기 자동 닫힘
```

## 상황별 통신 흐름 시나리오 및 예시 데이터

```c
// 0. 부팅/초기 동기화
STM -> Qt : R\r\n
STM -> Qt : S:00000011\r\n

// 1. 차량 접근 및 1층 입차
STM -> Qt : D\r\n
Qt  -> STM : I:1234\r\n
STM -> Qt : P:1\r\n

// 2. 만차 시 입차 시도
STM -> Qt : D\r\n
Qt  -> STM : I:1234\r\n
STM -> Qt : F\r\n

// 3. 1층 차량 요금 정산 후 출차
Qt  -> STM : O:1\r\n
STM -> Qt : E:1\r\n

// 4. 수동 상태 동기화 요청
Qt  -> STM : ?\r\n
STM -> Qt : S:00000000\r\n
```

```c
Qt → STM
P
E:<floor>
?

STM → Qt
D
P:<floor>
E:<floor>
S:<8bit>
```



# 소프트웨어 상세 설계

## 구현 함수 목록

### 1. 시스템 역할 및 책임 (Separation of Concerns)

#### [주요 전역 변수 및 자료구조]

```
hardware/
├── header/
│   ├── main.h             // 시스템 전역 설정, 핀 정의, 기본 헤더
│   └── parking_hw.h       // 서보/초음파/LED/슬롯 제어 함수 원형 선언
└── Src/
    ├── main.c             // 시스템 초기화 및 메인 제어 루프
    ├── uart.c             // UART 수신/송신 인터럽트 및 명령 파서
    ├── servo_motor.c      // 차단기 PWM 제어 및 자동 닫힘 타이머
    ├── ultrasonic.c       // 초음파 센서 거리 측정 로직
    └── led.c              // 8개 층 점유 LED 제어
```

#### [주요 클래스 및 멤버 변수/함수]

```
ParkingSystem/
├── ParkingSystem.pro      // 프로젝트 설정 (QT += core gui serialport sql widgets)
├── main.cpp               // QApplication 실행 진입점
├── mainwindow.ui          // UI 폼 (대시보드, 층별 상태, 차량번호 입력, 결제창)
├── mainwindow.h           // 메인 윈도우 클래스 선언
├── mainwindow.cpp         // UI 이벤트 핸들러, 비즈니스 로직, 요금 계산, DB 처리
├── serialworker.h         // 비동기 시리얼 통신 Worker 클래스 헤더
└── serialworker.cpp       // QSerialPort 비동기 데이터 송수신 구현
```

```c
// 8개 슬롯의 점유 상태 (Bit 0 = 1층, ..., Bit 7 = 8층 / 1: 점유, 0: 빈자리)
uint8_t g_slot_occupancy_mask = 0x00;

// 초음파 센서 관련 변수
uint16_t g_measured_distance_cm = 0;
uint8_t g_car_detected_flag = 0;

// 차단기 동작 타이머 (3초 딜레이 제어)
uint32_t g_barrier_timer = 0;
uint8_t g_is_barrier_open = 0;

// UART 버퍼
char g_rx_buffer[64];
uint8_t g_rx_index = 0;
```



## STM32_M4 변수 및 함수 정의

| 소스파일 | 타입 | 변수명 | 초기값 |
| --- | --- | --- | --- |
| `led_slot.c` | `static unsigned int` | parking_tower | 0b00000000 |

| 소스파일 | 타입(반환) | 함수원형 | 매개변수 | 상세동작설명 |
| --- | --- | --- | --- | --- |
| `main.c` | `int` | main(void) | x | 하드웨어 초기화 후 메인 루프에서 초음파 감지 및 차단기 타이머 폴링 |
| `ultrasonic.c` |  | Ultrasonic_ReadDistance(void) | x | PB4로 10µs 펄스 발사 후, PB5 Echo 핀의 High 유지 시간을 측정하여 거리(cm) 환산 반환 |
| `led.c` |  | LED_SetFloor(uint8_t floor, uint8_t state) | uint8_t floor, uint8_t state | 특정 층(1~8)의 비트를 세트/클리어하고 해당 GPIO 핀 점등(1)/소등(0) |
|  |  | LED_Init(void) | x | LED 초기화 |
| `led_slot.c` | `int` | Slot_FindLowestEmpty() | x | 가장 낮은 층부터 비어있는 층 반환 |
|  |  | Slot_SetOccupied(unsigned int floor) | unsigned int floor | 입차시 해당 층 슬롯 1 설정 |
|  |  | Slot_SetEmpty(unsigned int floor) | unsigned int floor | 출차시 해당 층 슬롯 0설정 |
|  | `unsigned int` | get_slot() | x | 전체 층 슬롯 반환 |
|  | `int` | error_input(int floor) | int floor |  |
| `servo_motor.c` |  | Servo_OpenBarrier(void) | x | TIM2_CH1 PWM 펄스를 90°(약 2.0ms)로 변경하여 차단봉 개방 |
|  |  | init_servo_motor(void) | x | 서보모터 초기설정 |
|  |  | Servo_CloseBarrier(void) | x | TIM2_CH1 PWM 펄스를 0°(약 1.0ms)로 변경하여 차단봉 하강 |
| `uart.c` |  | UART2_Init | uint32_t pclk1_hz, uint32_t baud | GPIOA(PA2-TX, PA3-RX)를 AF7(USART2)로 설정하고 전송 속도(BRR) 및 수신 인터럽트(RXNEIE)를 활성화한다. |
|  |  | UART2_SendChar | char c | USART2 송신 데이터 레지스터(TXE)가 빌 때까지 대기한 후 1바이트 문자를 전송한다. |
|  |  | UART2_SendString | const char *str | 널 문자(`\0`)를 만날 때까지 문자열을 구성하는 각 문자를 `UART2_SendChar`를 통해 순차 전송한다. |
|  | `bool` | UART2_IsCommandReady | x | 개행 문자 수신으로 단일 명령어가 버퍼에 준비되었는지 여부(`s_cmd_ready`)를 반환한다. |
|  |  | UART2_GetCommand | char *out_buf | 수신 완료된 명령어 버퍼 내용을 `out_buf`로 복사하고 수신 준비 플래그를 `false`로 리셋한다. |
|  |  | UART_ParseCommand | char *cmd | 수신 명령어(입차 `I`, 출차 `O:<floor>`, 조회 `?`)를 파싱하여 슬롯 상태 및 차단기를 제어하고 규격 응답(`P:<floor>`, `E:x`, `S:<mask >`, `E`)을 전송한다. |
|  |  | USART2_IRQHandler | x | USART2 RXNE 인터럽트 발생 시 수신 문자를 버퍼에 적재하고, 개행 문자(`\r`, `\n`) 감지 시 명령어 수신 완료 플래그를 활성화한다. |

## QT 변수 및 함수 정의

| **소스 파일** | **클래스** | **접근 지정자** | **타입 (반환 타입)** | **변수명 / 함수명** | **초기값 / 매개변수** |
| --- | --- | --- | --- | --- | --- |
| car.cpp | `Car` | `private` | QString | carNumber  | x |
|  | `Car` | `private` | int | floor | x |
|  | `Car` | `private` | QDateTime | entryTime  | x |
| parkingmanager.cpp | `ParkingManager` | `private` | QVector<Car> | cars  | x |
|  | `ParkingManager` | `private` | quint8 | slotStatus  | 0b00000000 |

| 소스 파일 | 클래스 | 접근 지정자 | 타입 | 함수명 | 매개변수 | 상세동작 설명 |
| --- | --- | --- | --- | --- | --- | --- |
| car.cpp | `Car` | `public` | 생성자 | Car | const QString &carNumber, int floor | 차량번호와 주차 층을 저장하고 현재 시간을 입차시간으로 설정한다. 출차시간은 초기화한다. |
|  | `Car` | `public` | 생성자 | Car | const QString &carNumber, int floor, const QDateTime &entryTime | 저장된 주차 기록 복원 시 사용한다. 차량번호, 주차 층, 기존 입차시간을 이용해 차량 객체를 생성한다. |
|  | `Car` | `public` | QString | getCarNumber | x | 차량번호를 반환한다. |
|  | `Car` | `public` | int | getFloor | x | 차량이 주차된 층을 반환한다. |
|  | `Car` | `public` | QDateTime | getEntryTime | x | 차량의 입차시간을 반환한다. |
|  | `Car` | `public` | QDateTime | getExitTime | x | 차량의 출차시간을 반환한다. |
|  | `Car` | `public` | void | setExitTime | const QDateTime &exitTime | 차량의 출차시간을 설정한다. |
|  |  |  |  |  |  |  |
| parkingmanager.cpp | `ParkingManager` | `public` | bool | addCar | const QString &carNumber, int floor | 차량번호와 배정된 층을 이용해 Car 객체를 생성하고 현재 주차 차량 목록에 추가한다. |
|  | `ParkingManager` | `public` | Car * | findCar | const QString &carNumber | 차량번호가 일치하는 현재 주차 차량을 검색하고 해당 객체의 포인터를 반환한다. |
|  | `ParkingManager` | `public` | QVector<Car *> | findCarByLastFour | const QString &lastFour | 차량번호 뒤 4자리가 일치하는 현재 주차 차량들을 검색하여 반환한다. |
|  | `ParkingManager` | `public` | bool | removeCar | const QString &carNumber | 출차 완료된 차량을 현재 주차 차량 목록에서 제거한다. |
|  | `ParkingManager` | `public` | qint64 | calculateParkingTime | const QString &carNumber, const QDateTime &exitTime | 차량의 입차시간과 출차시간 차이를 초 단위로 계산한다. |
|  | `ParkingManager` | `public` | int | calculateFee | qint64 parkingTime | 주차시간을 기준으로 주차요금을 계산한다. |
|  | `ParkingManager` | `public` | void | setSlotStatus | quint8 status | STM32에서 수신한 8비트 슬롯 점유 상태를 저장한다. |
|  | `ParkingManager` | `public` | quint8 | getSlotStatus | x | 현재 저장된 슬롯 점유 상태를 반환한다. |
|  | `ParkingManager` | `public` | bool | isFull | x | 8개의 주차 슬롯이 모두 사용 중인지 확인한다. |
|  | `ParkingManager` | `public` | int | getAvailableCount | x | 슬롯 상태를 이용해 현재 주차 가능한 슬롯 수를 계산한다. |
|  | `ParkingManager` | `public` | bool | isOccupied | int floor | 지정한 층이 현재 점유되어 있는지 확인한다. |
|  | `ParkingManager` | `public` | bool | saveParkingRecord | const QString &carNumber | 차량의 입차/출차 정보를 CSV 파일에 저장한다. |
|  | `ParkingManager` | `public` | void | loadParkingCars | x | 프로그램 시작 시 CSV 파일들을 읽어 출차하지 않은 차량의 Car 객체를 복원한다. |
|  | `ParkingManager` | `public` | quint8 | getStoredSlotStatus | x | CSV에서 복원된 차량들의 층 정보를 이용해 Qt 측의 8비트 슬롯 점유 상태를 생성한다. |
|  |  |  |  |  |  |  |
| serialmanager.cpp | `SerialManager` | `public` | 생성자 | SerialManager | QObject *parent | QSerialPort의 readyRead signal과 데이터 수신 함수를 연결한다. |
|  | `SerialManager` | `public` | bool | open | const QString &portName | 지정된 COM Port를 115200 baud, 8N1 조건으로 설정하고 UART 통신을 시작한다. |
|  | `SerialManager` | `public` | void | close | x | 열려 있는 Serial Port를 닫는다. |
|  | `SerialManager` | `public` | bool | isOpen | x | Serial Port의 연결 여부를 반환한다. |
|  | `SerialManager` | `public` | void | sendData | const QString &data | 문자열 뒤에 \r\n을 추가하여 UART로 송신한다. |
|  | `SerialManager` | `private` | void | receiveData | x | UART 수신 데이터를 버퍼에 저장하고 \r\n 단위로 하나의 명령을 분리한다. |
|  | `SerialManager` | `private` | void | parseReceivedData | const QByteArray &data | 수신된 UART 명령을 분석하여 READY, 슬롯 상태, 차량 감지, 입차 완료, 출차 완료 등의 signal을 발생시킨다. |
|  |  |  |  |  |  |  |
| serialmanager.h | `SerialManager` | `signals` | signal | readyReceived | x | STM32의 READY 신호 R이 수신되었음을 MainWindow에 전달한다. |
|  | `SerialManager` | `signals` | signal | slotStatusReceived | quint8 status | STM32에서 수신한 8비트 슬롯 상태를 MainWindow에 전달한다. |
|  | `SerialManager` | `signals` | signal | detectedReceived | x | STM32에서 차량 접근 감지 D가 수신되었음을 전달한다. |
|  | `SerialManager` | `signals` | signal | parkedReceived | int floor | STM32의 입차 완료 신호와 배정된 주차 층을 전달한다. |
|  | `SerialManager` | `signals` | signal | exitedReceived | x | STM32의 출차 완료 신호 E가 수신되었음을 전달한다. |
|  |  |  |  |  |  |  |
| mainwindow.cpp | `MainWindow` | `public` | 생성자 | MainWindow | QWidget *parent | UI와 SerialManager signal/slot을 초기화하고 CSV에 저장된 현재 주차 차량 정보를 복원한다. |
|  | `MainWindow` | `private` | void | updateHomeStatus | x | ParkingManager의 슬롯 상태를 이용해 주차 가능 대수, 슬롯 이미지 및 홈 화면 버튼 상태를 갱신한다. |
|  | `MainWindow` | `private slot` | void | onSerialReady | x | STM32의 READY 신호를 수신하면 ? 명령을 송신하여 초기 슬롯 상태를 요청한다. |
|  | `MainWindow` | `private slot` | void | onSlotStatusReceived | quint8 stmStatus | STM32 슬롯 상태와 CSV 기반 슬롯 상태를 비교하고, 실제 슬롯 상태를 갱신한 후 홈 화면을 업데이트한다. |
|  | `MainWindow` | `private slot` | void | onDetectedReceived | x | 차량 접근 신호 D 수신 시 만차가 아니면 홈 화면의 입차 버튼을 활성화한다. |
|  | `MainWindow` | `private slot` | void | on_btnEntryRequest_clicked | x | 입력된 차량번호의 형식을 확인하고 STM32에 입차 요청 I를 송신한다. 중복 요청 방지를 위해 입차 요청 버튼을 비활성화한다. |
|  | `MainWindow` | `private slot` | void | onParkedReceived | int floor | STM32에서 배정 층과 입차 완료 신호를 받으면 차량 객체를 등록하고 입차 기록을 저장한 뒤 입차 완료 화면을 표시한다. |
|  | `MainWindow` | `private` | void | clearEntryData | x | 입차 차량번호 입력창, 입차 완료 정보 및 관련 UI 상태를 초기화한다. |
|  | `MainWindow` | `private slot` | void | on_btnEntryToHome_clicked | x | 입차 관련 데이터를 초기화하고 STM32에 ?를 전송하여 최신 슬롯 상태를 받은 후 홈으로 돌아가도록 한다. |
|  | `MainWindow` | `private slot` | void | on_btnExitSearch_clicked | x | 입력된 차량번호 뒤 4자리와 일치하는 현재 주차 차량을 검색하여 목록에 표시한다. |
|  | `MainWindow` | `private slot` | void | on_btnExitRequest_clicked | x | 선택한 차량의 출차시간, 주차시간 및 요금을 계산하고 정산 화면에 표시한다. |
|  | `MainWindow` | `private slot` | void | on_btnPaymentRequest_clicked | x | 출차시간을 확정하고 주차 기록을 저장한 뒤 해당 차량의 층 정보를 이용해 STM32에 출차 요청 O:층을 송신한다. |
|  | `MainWindow` | `private slot` | void | onExitedReceived | x | STM32의 출차 완료 E를 수신하면 차량 객체를 삭제하고 출차 완료 화면을 표시한다. 일정 시간 후 최신 슬롯 상태를 요청한다. |
|  | `MainWindow` | `private` | void | clearExitData | x | 선택 차량, 출차시간, 검색 결과, 정산 정보 및 관련 UI 상태를 초기화한다. |
|  | `MainWindow` | `private slot` | void | on_btnExitToHome_clicked | x | 출차 검색 정보를 초기화하고 최신 슬롯 상태를 요청하여 홈으로 돌아간다. |
|  | `MainWindow` | `private slot` | void | on_btnPaymentToExit_clicked | x | 진행 중이던 정산 정보를 초기화하고 출차 검색 화면으로 돌아간다. |
|  | `MainWindow` | `private slot` | void | on_btnPaymentToHome_clicked | x | 정산 관련 데이터를 초기화하고 최신 슬롯 상태를 요청하여 홈 화면으로 돌아간다. |