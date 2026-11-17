# SecAuth — Exemplo C++ para Windows

Aplicação de console para testar o cadastro com licença, o login e a consulta de sessão de um usuário do seu software.

## Configure a aplicação

1. Entre no [site da SecAuth](https://getsecauth.com/login) e abra a sua aplicação no [painel do cliente](https://panel.getsecauth.com/).
2. Em **Configurações** da aplicação, copie o **App ID** e a **Secret Key**.
3. Abra [config/config.h](config/config.h) e substitua os dois valores de preenchimento pelos dados da sua aplicação.

```cpp
#pragma once

namespace ClientSettings {
    constexpr char Server[]    = "https://api.getsecauth.com";
    constexpr char AppId[]     = "SEU_APP_ID";
    constexpr char SecretKey[] = "SUA_SECRET_KEY";
}
```

Use a URL oficial como está. O cliente monta os caminhos da API. Mantenha os valores reais em uma cópia privada e use os campos de preenchimento ao compartilhar o projeto ou fazer commits.

## Compile e execute

Abra `SecAuth-Exemplo.sln` no Visual Studio com o workload **Desenvolvimento para desktop com C++** e o Windows SDK. Selecione **Release | x64**, compile a solução e execute `bin/x64/Release/SecAuth-Exemplo.exe`. O exemplo usa WinHTTP, bcrypt e advapi32 do Windows.

Gere uma licença disponível no painel da aplicação para testar o primeiro acesso.

| Opção | O que acontece |
| --- | --- |
| **1 · Registar licença** | Informe usuário, e-mail, senha de pelo menos 8 caracteres e licença. A API cria o usuário e vincula a licença ao dispositivo. |
| **2 · Iniciar sessão** | Informe o mesmo usuário e senha no dispositivo vinculado. O cliente mostra o estado da sessão e a validade da licença. |
| **3 · Estado da sessão** | Consulte a sessão atual e verifique se a licença ainda está válida. |
| **4 · Terminar sessão** | Encerre a sessão neste cliente. |
| **0 · Sair** | Feche o programa. |

O console limpa a tela ao abrir uma opção e ao exibir seu resultado. Pressione Enter para voltar ao menu.

## Integre ao seu software

Use [src/main.cpp](src/main.cpp) como referência para criar o cliente e [src/client.h](src/client.h) e [src/client.cpp](src/client.cpp) para integrar as chamadas. Vincule `winhttp.lib`, `bcrypt.lib` e `advapi32.lib`.

O exemplo usa a função `Credential` de `main.cpp` para derivar a credencial de desktop com HMAC-SHA256 a partir da Secret Key e do App ID. O construtor de `SecAuth::Client` recebe essa credencial derivada.

```cpp
// Trecho para usar dentro do fluxo de autenticação de main.cpp.
const std::string appId = Lower(ClientSettings::AppId);
SecAuth::Client client(Wide(ClientSettings::Server), appId,
    Credential(appId, ClientSettings::SecretKey));

// Cadastre uma vez com uma licença disponível.
if (firstAccess) {
    client.Register(username, email, password, license);
}

// Libere o acesso quando Login retornar com sucesso.
SecAuth::Profile profile = client.Login(username, password);
```

Enquanto o software estiver em uso, chame `client.Refresh()` periodicamente e trate falhas de sessão antes de liberar novas operações. Chame `client.Logout()` ao sair. As chamadas de rede são bloqueantes. Em um aplicativo com interface, use uma thread de trabalho e serialize as chamadas de cada instância.

[Guia de integração da SecAuth](https://getsecauth.com/integracoes)
