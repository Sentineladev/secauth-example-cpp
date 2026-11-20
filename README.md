# SecAuth — Exemplo C++ para Windows

Exemplo de integração com a API desktop **v2** do SecAuth. O projeto utiliza autenticação por conta + licença e **não incorpora a Secret Key mestra no cliente ou executável**.

## Configurar

1. Entre em https://getsecauth.com/login e abra a aplicação em https://panel.getsecauth.com/.
2. Copie somente o **App ID**, que é um identificador público. Não copie a Secret Key para o código ou para o executável.
3. Em [config/config.h](config/config.h), ajuste o App ID; mantenha a URL oficial da API.

```cpp
#pragma once
namespace ClientSettings {
    constexpr char Server[] = "https://api.getsecauth.com";
    constexpr char AppId[] = "SEU_APP_ID";
}
```

**Compatibilidade:** esta versão chama `/api/desktop/v2/register`, `/login`, `/session` e `/logout`. Essas rotas precisam estar publicadas no backend SecAuth. Os endpoints antigos `/api/desktop/*` continuam disponíveis para clientes legados, mas o exemplo atualizado não envia `X-Desktop-Key`.

## Compilar e executar

Abra `SecAuth-Exemplo.sln` no Visual Studio com *Desenvolvimento para desktop com C++* e o Windows SDK. Selecione **Release | x64**, compile e execute `bin/x64/Release/SecAuth-Exemplo.exe`.

Não é necessário incluir credenciais privadas no projeto ou enviar segredos ao comprador. Crie uma licença de teste válida no painel para verificar o cadastro. O sistema usa WinHTTP, bcrypt e advapi32 do Windows.

| Opção | Comportamento |
|---|---|
| 1 · Registar licença | Cadastra usuário, e-mail, senha e licença para o dispositivo |
| 2 · Iniciar sessão | Autentica por usuário, senha, App ID e dispositivo |
| 3 · Estado da sessão | Confirma validade da sessão e da licença |
| 4 · Terminar sessão | Solicita revogação da sessão ao backend e remove o token local |
| 0 · Sair | Encerra o exemplo |

Após login, o cliente renova sua sessão automaticamente a cada cinco minutos enquanto o aplicativo estiver em execução. Em falha transitória, tenta novamente após 30 segundos. Erros de autenticação definitivos invalidam a sessão local.

O token de acesso fica em memória, sem persistência em arquivos. O logout é revogável no backend com uma lista de sessões encerradas; se a rede não permitir a confirmação, o exemplo avisa que somente a sessão local foi apagada.

## Integração em outro programa

Os arquivos são [src/client.h](src/client.h), [src/client.cpp](src/client.cpp) e [src/main.cpp](src/main.cpp). O cliente usa chamadas bloqueantes; realize login e chamadas de registro fora da thread de interface. A thread de renovação já serializa as requisições com as operações principais.

```cpp
const std::string appId = Lower(ClientSettings::AppId);
SecAuth::Client client(Wide(ClientSettings::Server), appId);
client.Register(username, email, password, license);
SecAuth::Profile profile = client.Login(username, password);
```

**Importante:** nenhuma proteção de client-side torna o código à prova de engenharia reversa, modificação ou spoofing de identificadores de hardware. O backend deve validar licenças, identidade, bloqueios e permissões em cada operação sensível. Não utilize a Secret Key mestra ou outros segredos globais em binários distribuídos.

## Limitações e testes

Este repositório contém a implementação e configuração do exemplo, **não um binário compilado ou uma certificação de segurança**. Faça testes em um ambiente controlado com licença e usuário descartáveis, incluindo cadastro, login, renovação, logout, login após logout, chave rotacionada, licença expirada e falha temporária de rede.

Se a API v2 não estiver publicada, o exemplo atualizado não conseguirá autenticar; confirme a versão do backend antes de distribuir o executável.
