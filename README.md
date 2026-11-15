# SecAuth — Exemplo de Cliente C++

Exemplo de integração de um software Windows com a plataforma **SecAuth**, usando apenas uma aplicação de consola e a API do Windows.

O exemplo não usa bibliotecas de interface gráfica: é uma consola simples, com menu, para mostrar o fluxo de autenticação.

## O que o exemplo faz

Ao executar, aparece um menu:

```
  [1] Registar licença
  [2] Iniciar sessão
  [3] Estado da sessão
  [4] Terminar sessão
  [0] Sair
```

- **Registar licença** — vincula uma licença a um usuário e a este dispositivo
- **Iniciar sessão** — autentica o usuário e abre a sessão
- **Estado da sessão** — consulta a sessão atual
- **Terminar sessão** — encerra a sessão

## Configuração

Em `config/config.h`, preencha as credenciais da sua aplicação:

```cpp
namespace ClientSettings {
constexpr char Server[]    = "https://seu-servidor";
constexpr char AppId[]     = "SEU_APP_ID";                 // 24 caracteres hexadecimais
constexpr char SecretKey[] = "SUA_SECRET_KEY";             // 64 caracteres hexadecimais
}
```

O `AppId` e a `SecretKey` estão no painel, em **Aplicação → Configurações**.

> A `SecretKey` só é usada durante a compilação, para derivar a credencial do cliente. Ela nunca é enviada pela rede.

## Compilação

1. Abra `SecAuth-Exemplo.sln` no **Visual Studio**
2. Selecione a configuração **Release | x64**
3. Compile o projeto

Nenhuma dependência externa precisa de ser instalada.

## Integração no seu projeto

Copie estes ficheiros para o seu projeto:

- `src/client.h`
- `src/client.cpp`
- `third_party/json.hpp`

Inclua `third_party` no caminho de cabeçalhos e vincule as bibliotecas:

```
winhttp.lib
bcrypt.lib
advapi32.lib
```

Uso básico:

```cpp
#include "client.h"

SecAuth::Client client(
    L"https://seu-servidor",
    "SEU_APP_ID",
    "CREDENCIAL_DERIVADA"
);

// Criar a conta e vincular a licença (apenas no primeiro acesso)
client.Register("usuario", "email@exemplo.com", "palavra-passe", "SUA-LICENCA");

// Autenticar antes de liberar o seu software
SecAuth::Profile profile = client.Login("usuario", "palavra-passe");

// Consultar a sessão periodicamente
profile = client.Refresh();

// Encerrar a sessão
client.Logout();
```

> As operações de rede são bloqueantes. Execute-as numa thread de trabalho, serializadas para cada instância.

## Licenças de terceiros

- [nlohmann/json](https://github.com/nlohmann/json) — MIT
