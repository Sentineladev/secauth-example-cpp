# SecAuth — Exemplo C++ para Windows

Cliente de demonstração da API desktop v2 com **credencial revogável por rotação da Secret Key**, cadastro, login, renovação e logout. O executável **não recebe a Secret Key mestra**.

## Configuração para Visual Studio

1. Entre no painel da aplicação em https://panel.getsecauth.com e abra **Configurações**.
2. Copie o **App ID** e escolha **Copiar credencial do Client C++**. Esta ação exige sua senha e código 2FA. A credencial é derivada no servidor da Secret Key atual.
3. Configure `config/config.h` antes de compilar:

```cpp
#pragma once
namespace ClientSettings {
constexpr char Server[] = "https://api.getsecauth.com";
constexpr char AppId[] = "SEU_APP_ID";
constexpr char AppName[] = "MeuApp";
constexpr char Version[] = "1.0.0";
}

#define SECAUTH_DESKTOP_KEY "COLE_A_CREDENCIAL_DESKTOP_DE_64_CARACTERES"
```

4. Configure `AppName` como o nome exibido no terminal e `Version` para a **mesma versão exigida no painel** (Configurações → Versão exigida do Client C++). Alterar uma versão no painel ou no executável sem atualizar a outra faz o servidor rejeitar login, cadastro e renovação. App Name é apenas identificação visual, não credencial de segurança.
5. Abra `SecAuth-Exemplo.sln` e compile **Release | x64**. Só use a credencial de uma aplicação que você administra; não versione valores reais.

## Como a revogação funciona

Todas as chamadas desktop v2 de cadastro, login e sessão enviam `X-Desktop-Key` via HTTPS. O backend compara o valor com a credencial derivada da Secret Key mestra atual. Ao usar **Resetar Secret Key**, a credencial antiga deixa de ser aceita e um binário compilado com ela não consegue registrar, fazer novo login ou renovar a sessão. Para habilitar outra versão, copie a nova credencial no painel e recompile o cliente.

Depois do login, a sessão é verificada aproximadamente a cada **30 segundos**. A função `IsLoggedIn()` fica falsa após 90 segundos sem validação bem-sucedida, evitando continuar confiando indefinidamente numa conexão interrompida. Uma sessão invalidada por rotação perde o acesso quando o servidor a verifica. O software integrador deve verificar `IsLoggedIn()` antes de liberar funcionalidades protegidas, além de tratar erros de `Refresh()`.

As quatro opções do exemplo são registrar licença, iniciar sessão, consultar a sessão e encerrar a sessão. O logout solicita revogação no servidor e limpa o token local.

## Sobre a obfuscação skCrypt

Este exemplo inclui `include/skStr.h` fornecido pelo proprietário e utiliza `skCrypt` numa macro com literal de string para ofuscar a credencial desktop compilada. A credencial não fica adicionalmente declarada como variável global constexpr em texto simples. A biblioteca ofusca strings com XOR durante a compilação. **Não é criptografia que impeça engenharia reversa:** pessoas que possuem o binário podem recuperar a credencial desktop. Ela também não deve receber a Secret Key mestra. A proteção fundamental é o servidor validar sessão, licença, dispositivo e a credencial vigente em cada operação.

A credencial desktop é reutilizável pelo cliente e deve ser tratada como material sensível. Sua rotação invalida os binários anteriores, mas não torna um software distribuído inviolável.

## Testar a revogação

1. Preencha o App ID e a credencial desktop vigente, compile e faça login com uma conta e licença de teste.
2. No painel, redefina a Secret Key com senha e 2FA.
3. No cliente já aberto, consulte o estado da sessão ou aguarde a verificação automática; a sessão antiga deverá ser negada.
4. Tente fazer login novamente com o mesmo executável antigo; deverá falhar.
5. Revele e copie a **nova credencial desktop**, recompile com a versão atual exigida no painel e teste novo login; deverá funcionar.

**Compatibilidade:** o backend mantém os endpoints legados `/api/desktop/*`, porém o exemplo atualizado usa `/api/desktop/v2/*`. As alterações no protocolo v2 precisam estar implantadas para esse binário funcionar.

## Gestão da versão

A configuração contém `Server`, `AppId`, `AppName` e `Version`. O cliente envia `X-Client-Version` nas requisições desktop v2; o servidor exige correspondência exata com a versão da aplicação. A versão necessária pode ser alterada no painel após confirmação. Quando o servidor mudar de versão, builds com a versão anterior passam a receber `426` nas verificações da sessão, mesmo que a Secret Key permaneça igual.

`AppName` é exibido no menu. Ele não dá acesso à API nem é usado como segredo; `AppId` identifica o software. `SECAUTH_DESKTOP_KEY` é a credencial desktop revogável derivada no servidor e ofuscada por `skCrypt` no build.

Não confunda `Version` com uma barreira criptográfica: um cliente modificado pode falsificar o cabeçalho de versão. Use a rotação da credencial desktop, verificações contínuas de licença e controles do backend como mecanismos reais de autorização.

## Validação do nome cadastrado

O campo `AppName` em `config/config.h` deve coincidir **exatamente** com o nome salvo em **Configurações** da aplicação no painel. O cliente calcula SHA-256 do nome em UTF-8 e envia somente a impressão digital em `X-Client-App-Fingerprint`. Quando o nome no painel muda, clientes C++ compilados com o nome anterior deixam de autenticar ou renovar a sessão e recebem uma mensagem indicando que precisam ser atualizados. A configuração do projeto já utiliza `/utf-8` para permitir nomes acentuados.

Os campos `AppName` e `Version` são metadados verificáveis, não segredos criptográficos; um executável modificado pode falsificá-los. A autorização deve continuar dependente da credencial desktop rotativa, login, licença e validações no servidor.
