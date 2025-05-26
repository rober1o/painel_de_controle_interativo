# PAINEL DE CONTROLE INTERATIVO PARA BIBLIOTECA

O projeto tem como finalidade criar um painel de controle interativo para uma biblioteca, monitorando a quantidade de pessoas permitidas em uma biblioteca, além de oferecer sinais visuais e sonoros, tornando-o mais acessível. O projeto utiliza conceitos de multitarefa com o FreeRTOS, além de mecanismos como mutex, semáforo binário e semáforo de contagem.

## Componentes Utilizados


1. **Botão Pushbutton**
2. **Display OLED 1306**
3. **Buzzer**
4. **Matriz de LED 5x5 WS2812** 
5. **Led RGB**

## Funcionalidade

Sistema foi projetado para operar com três tarefas principais: uma responsável pela entrada de usuários, outra pela saída, e uma terceira dedicada ao reset do sistema.
Ao ser inicializado, o sistema exibirá um painel no display OLED contendo o número de usuários atualmente ativos na biblioteca, começando com o valor inicial de zero. Simultaneamente, o LED RGB será aceso na cor azul, indicando que a biblioteca encontra-se completamente desocupa-da.


A interação do usuário com o sistema é realizada por meio de três botões:

•	Botão A: incrementa a quantidade de usuários
•	Botão B: Decrementa a quantidade de usuarios
•	Botão C (Joystick): Reseta o sistema

Indicações Visuais — LED RGB:

•	LED Azul: Biblioteca desocupada.
•	LED Verde: Entre 1 e MAX-2 usuários ativos.
•	LED Amarelo: Apenas uma vaga restante.
•	LED Vermelho: Nenhuma vaga disponível.


Indicações Visuais — Matriz de LEDs:

•	"X" vermelho: Exibido quando um usuário tenta acessar a biblioteca sem que haja vagas disponíveis.
•	Exclamação amarela (!): Exibida quando o sistema é resetado.

Sinais Sonoros — Buzzer:

•	1 beep curto: Indica que um usuário tentou entrar na biblioteca quando não havia vagas dis-poníveis.
•	2 beeps curtos: Indicam que o sistema foi resetado com sucesso.


### Como Usar

#### Usando a BitDogLab

- Clone este repositório: git clone https://github.com/rober1o/painel_de_controle_interativo.git;
- Usando a extensão Raspberry Pi Pico importar o projeto;
- Ajuste o diretório do FreeRTOS, conforme seu computador
- Compilar o projeto;
- Plugar a BitDogLab usando um cabo apropriado e gravar o código.

## Demonstração

<!-- TODO: adicionar link do vídeo -->
