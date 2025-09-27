#include <iostream>
#include <memory>
#include <string>
#include <grpcpp/grpcpp.h>

#include "urls.pb.h"
#include "urls.grpc.pb.h"

using grpc::Channel;
using grpc::ClientContext;
using grpc::Status;
using encurtador::EncurtadorURL;
using encurtador::RequisicaoEncurtar;
using encurtador::RespostaEncurtar;
using encurtador::RequisicaoObter;
using encurtador::RespostaObter;

class EncurtadorURLClient {
public:
    EncurtadorURLClient(std::shared_ptr<Channel> channel)
        : stub_(EncurtadorURL::NewStub(channel)) {}

    // Chama o método EncurtarURL
    std::string Encurtar(const std::string& url_longa) {
        RequisicaoEncurtar request;
        request.set_url_longa(url_longa);

        RespostaEncurtar reply;
        ClientContext context;

        Status status = stub_->EncurtarURL(&context, request, &reply);

        if (status.ok()) {
            return reply.url_curta();
        } else {
            std::cerr << "Erro no RPC EncurtarURL: " << status.error_message() << std::endl;
            return "";
        }
    }

    // Chama o método ObterURLLonga
    std::string Obter(const std::string& codigo_curto) {
        RequisicaoObter request;
        request.set_codigo_curto(codigo_curto);

        RespostaObter reply;
        ClientContext context;

        Status status = stub_->ObterURLLonga(&context, request, &reply);

        if (status.ok()) {
            return reply.url_longa();
        } else {
            std::cerr << "Erro no RPC ObterURLLonga: " << status.error_message() << std::endl;
            return "";
        }
    }

private:
    std::unique_ptr<EncurtadorURL::Stub> stub_;
};

int main() {
    // Conecta ao servidor Python
    EncurtadorURLClient client(grpc::CreateChannel(
        "localhost:50051", grpc::InsecureChannelCredentials()));

    // 1. Encurtar uma URL longa de exemplo
    std::string url_longa = "https://www.alcides.com/artigos/introducao-grpc";
    std::string url_curta = client.Encurtar(url_longa);

    std::cout << "URL Longa: " << url_longa << std::endl;
    std::cout << "URL Curta gerada: " << url_curta << std::endl;

    // Extrai o código curto da URL curta (parte após a última '/')
    std::string codigo_curto;
    size_t pos = url_curta.find_last_of("/");
    if (pos != std::string::npos) {
        codigo_curto = url_curta.substr(pos + 1);
    }

    // 2. Obter a URL longa original a partir do código
    std::string url_recuperada = client.Obter(codigo_curto);
    std::cout << "URL Recuperada: " << url_recuperada << std::endl;

    return 0;
}
