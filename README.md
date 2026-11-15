# SecAuth — Exemplo de Cliente C++

Exemplo de integração de um cliente desktop com a plataforma **SecAuth**, usando **ImGui + DirectX 11**.

## Como funciona

O cliente se comunica com a API do SecAuth para:

- **Cadastro** (`Register`) — vincula uma licença a um usuário e dispositivo
- **Login** (`Login`) — autentica o usuário e inicia uma sessão
- **Atualização** (`Refresh`) — renova a sessão periodicamente
- **Logout** (`Logout`) — encerra a sessão

A sessão é verificada automaticamente a cada minuto e ao expirar.

## Configuração

Em `config/config.h`, preencha as credenciais da sua aplicação:

```cpp
namespace ClientSettings {
constexpr char Server[]    = "https://seu-servidor";       // URL da sua aplicação
constexpr char AppId[]     = "SEU_APP_ID";                 // App ID (24 caracteres hex)
constexpr char SecretKey[] = "SUA_SECRET_KEY";             // Chave mestra (64 caracteres hex)
}
```

> A `SecretKey` é usada apenas na compilação para derivar a credencial de conexão. Ela nunca é enviada pela rede.

## Compilação

1. Abra `SecAuth-Exemplo.sln` no **Visual Studio**
2. Selecione a configuração **Release | x64**
3. Compile o projeto

## Integração no seu projeto

Copie os seguintes arquivos para o seu projeto:

- `src/client.h`
- `src/client.cpp`
- `third_party/json.hpp`

Inclua a pasta `third_party` no caminho de cabeçalhos e vincule as bibliotecas:

```
winhttp.lib
bcrypt.lib
advapi32.lib
```

Uso básico:

```cpp
#include "client.h"

SecAuth::Client client(
    "https://seu-servidor",
    "SEU_APP_ID",
    "credencial_derivada"
);

// Cadastrar um usuário com uma licença
client.Register("usuario", "senha", "SUA-LICENCA");

// Fazer login
client.Login("usuario", "senha");

// Renovar a sessão (chame periodicamente)
client.Refresh();

// Encerrar a sessão
client.Logout();
```

> As operações de rede são bloqueantes. Execute-as em uma thread de trabalho, serializadas para cada instância.

## Licenças de terceiros

- [Dear ImGui](https://github.com/ocornut/imgui) — MIT
- [nlohmann/json](https://github.com/nlohmann/json) — MIT
