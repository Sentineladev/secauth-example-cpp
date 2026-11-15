# SecAuth Exemplo

1. Inicie o backend do Sec-Auth-V5. No site, crie uma aplicação ativa e gere uma licença.
   A aplicação precisa pertencer a um cliente (subscriber) ativo: se a conta dona for excluída ou banida, o servidor responde `Aplicação indisponível.` para todas as chamadas de cadastro, login e sessão. Se isso acontecer, reaponte a aplicação para um cliente ativo no painel ou crie uma nova aplicação.
2. O `config/config.h` já vem preenchido com uma **conta de demonstração** pública para você testar imediatamente. Para usar a sua própria aplicação, troque `Server`, `AppId` e `SecretKey` pelos valores de **Aplicação → Configurações** no site. Para testar nesta máquina, use `http://127.0.0.1:3000` em `Server`.
3. Abra `SecAuth-Exemplo.sln` no Visual Studio e compile em **Release | x64**. O projeto usa skStr.h para ofuscar a URL e o App ID no executável durante a compilação.
4. Abra o executável, cadastre um usuário com a licença e entre com o nome de usuário e a senha.
5. No painel da aplicação, use **Usuários → Banir** ou **Licenças → Banir**. O cliente recusa o próximo login e volta à tela de login com o motivo na próxima verificação da sessão.

O cliente verifica a sessão a cada minuto e ao chegar ao fim da validade. O servidor responde com o nome de usuário, a ativação em UTC, a expiração e o tempo restante. O cadastro desktop vincula uma licença à conta e ao dispositivo em uma transação MongoDB. Use um cluster com suporte a transações, como o Atlas.

Para integrar, adicione `src/client.h`, `src/client.cpp` e `third_party/json.hpp` e inclua `third_party` no caminho de cabeçalhos. Vincule `winhttp.lib`, `bcrypt.lib` e `advapi32.lib`. Crie um `SecAuth::Client` com a URL do servidor, o App ID e a credencial derivada pelo script de build e chame `Register`, `Login`, `Refresh` e `Logout`. As operações de rede são bloqueantes. Execute-as em uma thread de trabalho, serializadas para cada instância. Trate uma falha de `Refresh` como perda de acesso.

Para distribuir, configure o endereço **HTTPS** público e o ID da aplicação em `config/config.h`, recompile e entregue o executável. No servidor, termine TLS em um proxy que sobrescreva os cabeçalhos encaminhados e mantenha a porta HTTP interna acessível apenas ao proxy. O build deriva a credencial de conexão a partir da chave mestra e ofusca a credencial, a URL e o App ID com skStr.h. Guarde o config/config.h na máquina de compilação. Esses valores podem ser extraídos por análise do executável. Após resetar a Secret Key, copie a nova chave mestra para `SecretKey` e recompile. A sessão anterior falha na próxima verificação. `JWT_SECRET`, segredos das aplicações e credenciais de e-mail ficam no servidor. O cliente usa WinHTTP, valida os certificados do Windows e guarda o token de sessão em memória.

O projeto foi compilado com o Visual Studio instalado nesta máquina. A solução inclui configuração para o toolset v140, que precisa ser validada em uma instalação com esse compilador.

A biblioteca JSON incluída é [nlohmann/json 3.11.3](https://github.com/nlohmann/json/releases/tag/v3.11.3), licenciada sob MIT conforme indicado no cabeçalho.
