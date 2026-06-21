# Controle de Velocidade de Motor DC com Encoder e PID

## Projeto Final - Disciplina de Microcontroladores

* **Autores:** Matheus Pedro da Silva, Nelson Alves Sousa Moreira, Kauã Lessa Lima dos Santos
* **Disciplina:** Microcontroladores e suas Aplicações 
* **Hardware:** Arduino UNO 
* **Data:** Junho/2026

---

## 📋 Sumário

- [Descrição do Projeto](#-descrição-do-projeto)
- [Objetivos](#-objetivos)
- [Lista de Materiais](#-lista-de-materiais)
- [Diagrama de Conexões](#-diagrama-de-conexões)
- [Funcionamento do Sistema](#-funcionamento-do-sistema)
  - [Leitura do Encoder](#leitura-do-encoder)
  - [Controle PID](#controle-pid)
  - [Interface com o Usuário](#interface-com-o-usuário)
- [Configuração do PID](#-configuração-do-pid)
- [Como Usar](#-como-usar)
- [Resultados Obtidos](#-resultados-obtidos)
- [Conclusão](#-conclusão)
- [Referências](#-referências)

---

## 📝 Descrição do Projeto

Este projeto consiste em um **sistema de controle de velocidade** para um motor DC com encoder, utilizando um **controlador PID** implementado em um **Arduino UNO**. O sistema permite:

- Definir a velocidade desejada (setpoint) através de um **potenciômetro**
- Medir a velocidade real do motor através de um **encoder Hall**
- Ajustar automaticamente a velocidade usando um **controle PID** para manter o setpoint mesmo sob variações de carga
- Exibir a velocidade alvo e real em um **display LCD 16x2 com interface I2C**
- Monitorar os dados em tempo real via **Serial Plotter**

O sistema é alimentado por **8 pilhas AA (2 cases de 4 pilhas em série)**, fornecendo **12V** para a ponte H, garantindo potência suficiente para o motor.

[![Assista ao vídeo do projeto](https://img.youtube.com/vi/Af0YiHf767o/maxresdefault.jpg)](https://youtu.be/Af0YiHf767o)

---

## 🎯 Objetivos

### Objetivo Geral
Desenvolver um sistema de controle de velocidade para motor DC com realimentação por encoder, utilizando um controlador PID para garantir precisão e estabilidade.

### Objetivos Específicos
- Implementar a leitura do encoder via interrupções externas
- Projetar e ajustar os ganhos do controlador PID (Kp, Ki, Kd)
- Desenvolver uma interface amigável com LCD para exibição de dados
- Criar um sistema de monitoramento via Serial Plotter
- Garantir que o motor mantenha a velocidade desejada mesmo com variações de carga

---

## 📦 Lista de Materiais

| Componente | Quantidade | Especificação |
|------------|------------|---------------|
| Arduino Mega 2560 | 1 | Microcontrolador ATmega2560 |
| Motor DC com Encoder | 1 | Motor com encoder Hall (C1 e C2) |
| Ponte H TB6612FNG | 1 | Driver para motor DC |
| Display LCD I2C | 1 | 16x2 caracteres, endereço 0x27 |
| Potenciômetro | 1 | 10kΩ linear |
| Case de 4 Pilhas | 2 | Para alimentação do motor (12V total) |
| Pilhas AA | 8 | 1.5V cada |
| Jumpers | Vários | Macho-Macho e Macho-Fêmea |
| Protoboard | 1 | 400 ou 830 pontos |
| Cabo USB | 1 | Para programação e comunicação serial |

---

## 🔌 Diagrama de Conexões

<img width="1140" height="781" alt="image" src="https://github.com/user-attachments/assets/fca692eb-94d8-4e54-a70a-c1829ee52eba" />


### Conexões do Motor (TB6612FNG)

| Pino TB6612 | Pino Arduino | Descrição |
|-------------|--------------|-----------|
| PWMA | D9 (PWM) | Controle de velocidade (PWM) |
| AIN1 | D7 | Controle de direção 1 |
| AIN2 | D8 | Controle de direção 2 |
| STBY | 5V | Standby ativo |
| VM | +12V (Pilhas) | Alimentação do motor |
| VCC | 5V | Alimentação lógica |
| GND | GND | Massa comum |
| AO1 | Motor M1 | Saída para o motor |
| AO2 | Motor M2 | Saída para o motor |

### Conexões do Encoder

| Pino Encoder | Pino Arduino | Descrição |
|--------------|--------------|-----------|
| 3.3V | 3.3V | Alimentação do encoder |
| GND | GND | Massa |
| C1 | D2 (INT0) | Canal 1 (interrupção) |
| C2 | D3 (INT1) | Canal 2 (interrupção) |

### Conexões dos Periféricos

| Periférico | Pino Arduino | Descrição |
|------------|--------------|-----------|
| Potenciômetro (esquerda) | GND | Referência negativa |
| Potenciômetro (meio) | A0 | Sinal analógico |
| Potenciômetro (direita) | 5V | Referência positiva |
| LCD I2C (GND) | GND | Massa |
| LCD I2C (VCC) | 5V | Alimentação |
| LCD I2C (SDA) | D20 | Dados I2C |
| LCD I2C (SCL) | D21 | Clock I2C |

---

## 🧠 Funcionamento do Sistema

### Leitura do Encoder

O encoder possui dois canais (C1 e C2) que geram pulsos conforme o motor gira. O código utiliza **interrupções externas** (INT0 e INT1) para contar cada mudança de estado (borda de subida e descida), garantindo alta precisão na medição.

**Cálculo do RPM:**
```
Frequência = (Pulsos_contados) / (Tempo_de_medição_em_segundos)
RPM = (Frequência × 60) / (Pulsos_por_volta)
```

**Configuração do Encoder:**
- Pulsos por volta: **1496** (ajustável conforme o motor)
- Período de medição: **50ms** (20 medições por segundo)

### Controle PID

O controlador PID atua para manter a velocidade do motor igual ao valor definido pelo potenciômetro.

#### Algoritmo PID

O PID calcula a correção necessária baseada no erro entre o setpoint e o valor real:

```
erro = setpoint - real

integral = integral + erro × dt
derivativo = (erro - erro_anterior) / dt

saída = Kp × erro + Ki × integral + Kd × derivativo
```

#### Anti-Windup
Para evitar que o termo integral cresça indefinidamente (caso o motor não consiga atingir o setpoint), foi implementado um **anti-windup** que limita a integral entre -100 e 100.

### Interface com o Usuário

#### Display LCD I2C
- **Linha 1:** Mostra o **setpoint** (velocidade desejada)
- **Linha 2:** Mostra a **velocidade real** medida pelo encoder
- Atualização: **350ms** (evita flickering)

#### Monitor Serial
- Dados enviados via **Serial Plotter** para visualização gráfica:
  - `Alvo_RPM:` (setpoint)
  - `Real_RPM:` (velocidade medida)
  - `Erro:` (diferença entre alvo e real)

#### Potenciômetro
- Define o setpoint de 10 a 300 RPM
- Zona morta para valores abaixo de 10 (garante que o motor pare)

---

## ⚙️ Configuração do PID

Os ganhos foram ajustados experimentalmente para obter a melhor resposta:

| Ganho | Valor | Função |
|-------|-------|--------|
| **Kp** | 1.1 | Ação proporcional - corrige o erro imediato |
| **Ki** | 1.3 | Ação integral - elimina o erro em regime permanente |
| **Kd** | 0.01 | Ação derivativa - amortece oscilações |

### Como Ajustar os Ganhos (Método Ziegler-Nichols)

1. **Kp:** Aumente até o sistema oscilar, depois reduza pela metade
2. **Ki:** Comece com 0 e aumente até eliminar o erro estático
3. **Kd:** Adicione para reduzir overshoot (geralmente 10% do Kp)

---

## 🚀 Como Usar

### 1. Montagem do Hardware
1. Coloque todos os componentes na protoboard seguindo o diagrama de conexões
2. Conecte os dois cases de pilhas em série (PRETO-CASE1 no VERMELHO-CASE2)
3. Alimente o Arduino via USB
4. Conecte as pilhas ao VM do driver (após verificar a tensão com multímetro)

### 2. Configuração do Software
1. Abra o Arduino IDE
2. Instale a biblioteca: `LiquidCrystal I2C` (Frank de Brabander)
3. Selecione a placa: **Arduino Mega 2560**
4. Selecione a porta: **COM7** (ou a que aparecer no Gerenciador de Dispositivos)
5. Carregue o código para o Arduino

### 3. Operação
1. Gire o potenciômetro para definir a velocidade desejada
2. Observe o LCD para ver o setpoint e a velocidade real
3. Abra o Serial Plotter (Ferramentas → Serial Plotter) para visualizar os gráficos
4. O PID ajustará automaticamente o motor para manter a velocidade definida
---

## 📊 Resultados Obtidos

### Desempenho do PID
- **Tempo de estabilização:** ≈ 1 segundo
- **Erro em regime permanente:** < 4 RPM
- **Sobressinal:** < 10% (ajustável via Kd)

### Gráfico de Experimento

<img width="1200" height="800" alt="data_view_micro" src="https://github.com/user-attachments/assets/95ee9958-34fe-4ced-a3f9-902f5af8801d" />

---

## 🏁 Conclusão

O sistema desenvolvido atendeu todos os objetivos propostos, demonstrando:

- ✅ Controle preciso de velocidade usando PID
- ✅ Leitura confiável do encoder via interrupções
- ✅ Interface amigável com LCD e Serial Plotter
- ✅ Robustez contra variações de carga
- ✅ Código bem estruturado e comentado

O projeto proporcionou uma compreensão aprofundada sobre:
- Controle de sistemas embarcados
- Implementação de PID em microcontroladores
- Leitura de sensores com interrupções
- Comunicação I2C e exibição de dados

---

## 📚 Referências

1. **ARDUINO. Arduino UNO Datasheet.** Disponível em: https://docs.arduino.cc/hardware/uno-rev3/
2. **TOSHIBA. TB6612FNG Motor Driver Datasheet.** Disponível em: https://www.sparkfun.com/datasheets/Robotics/TB6612FNG.pdf
3. **OGATA, Katsuhiko. Engenharia de Controle Moderno.** 5ª ed. Pearson, 2011.
4. **BALBINO, Eduardo. Controle PID em Arduino.** Embarcados, 2020.
5. **FRANK DE BRABANDER. LiquidCrystal I2C Library.** GitHub, 2023.

