# Tarefa 1 - U4, C4 (Rotinas de Interrupção)

**Desenvolvido por:** Letícia Gonçalves Souza  

Este projeto foi desenvolvido como parte do programa **EmbarcaTech**, com o objetivo de aplicar conceitos avançados de **rotinas de interrupção (IRQ)**, **debouncing** e **controle de LEDs**. A atividade visa consolidar o entendimento prático desses conceitos através da implementação de funcionalidades interativas em um ambiente controlado.

---

## Sobre o Projeto

O projeto consiste em um sistema que utiliza o **Kit BitDogLab** para controlar LEDs e botões, aplicando técnicas de interrupção e debouncing. O foco principal é garantir que as interações com os botões sejam precisas e que os LEDs respondam de forma adequada, criando uma experiência visual interativa.

---


## Vídeo de Demonstração

[Assista ao vídeo de demonstração aqui](https://youtu.be/o9jb8JaN8So)
---

## Funcionalidades

1. **Piscar do LED RGB**:
   - O LED vermelho do LED RGB pisca continuamente a uma frequência de **5 vezes por segundo**.

2. **Incremento e Decremento com Botões**:
   - **Botão A**: Incrementa o número exibido na matriz de LEDs toda vez que é pressionado.
   - **Botão B**: Decrementa o número exibido na matriz de LEDs toda vez que é pressionado.

3. **Efeitos Visuais com LEDs WS2812**:
   - Os LEDs WS2812 são utilizados para criar efeitos visuais que representam números de **0 a 9**, proporcionando uma experiência visual dinâmica e interativa.

---

## Tecnologias e Ferramentas Utilizadas

- **Hardware**:
  - **Kit BitDogLab**:
    - Raspberry Pi Pico W
    - LEDs WS2812
    - Botões A e B
    - LED RGB

- **Software**:
  - Linguagem de programação: **C**
  - Ambiente de Desenvolvimento: **Visual Studio Code**

---

## Referências

[1] Aula síncrona ministrada pelo professor **Wilton Lacerda Silva**.

---

## Como Executar o Projeto

1. **Configuração do Ambiente**:
   - Certifique-se de ter o **Visual Studio Code** instalado e configurado para desenvolvimento em C.
   - Conecte o **Raspberry Pi Pico W** ao seu computador e configure-o para ser reconhecido pelo ambiente de desenvolvimento.

2. **Clonar o Repositório**:
   ```bash
   git clone https://github.com/Ltiicia/ContadorLed.git
   cd ContadorLed
