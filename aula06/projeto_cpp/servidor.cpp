#include <iostream>
#include <memory>
#include <string>
#include <vector>
#include <map>
#include <numeric>

#include <grpcpp/grpcpp.h>
#include "notas.grpc.pb.h"

// Usando um std::map para simular o banco de dados em memória
// A chave é "RA_CODDISCIPLINA", e o valor é um objeto Nota
std::map<std::string, gerencia_notas::Nota> db_notas_cpp;

class GerenciadorNotasImpl final : public gerencia_notas::GerenciadorNotas::Service {
public:
    grpc::Status AdicionarNota(grpc::ServerContext* context,
                               const gerencia_notas::AdicionaNotaRequest* request,
                               gerencia_notas::StatusResponse* response) override {
        std::string chave = request->ra() + "_" + request->cod_disciplina();
        if (db_notas_cpp.count(chave)) {
            response->set_sucesso(false);
            response->set_msg("Nota já existe. Use 'AlterarNota'.");
        } else {
            gerencia_notas::Nota nova_nota;
            nova_nota.set_ra(request->ra());
            nova_nota.set_cod_disciplina(request->cod_disciplina());
            nova_nota.set_ano(request->ano());
            nova_nota.set_semestre(request->semestre());
            nova_nota.set_nota(request->nota());
            db_notas_cpp[chave] = nova_nota;
            response->set_sucesso(true);
            response->set_msg("Nota adicionada com sucesso!");
        }
        return grpc::Status::OK;
    }

    grpc::Status AlterarNota(grpc::ServerContext* context,
                             const gerencia_notas::AdicionaNotaRequest* request,
                             gerencia_notas::StatusResponse* response) override {
        // cria a chave da mesma forma que em AdicionarNota
        std::string chave = request->ra() + "_" + request->cod_disciplina();

        // verifica se a nota existe no banco
        if (db_notas_cpp.count(chave)) {
            // atualiza os dados da nota existente
            db_notas_cpp[chave].set_ano(request->ano());
            db_notas_cpp[chave].set_semestre(request->semestre());
            db_notas_cpp[chave].set_nota(request->nota());

            response->set_sucesso(true);
            response->set_msg("Nota alterada com sucesso!");
        } else {
            response->set_sucesso(false);
            response->set_msg("Nota não encontrada. Use 'AdicionarNota'.");
        }

        return grpc::Status::OK;
    }

    grpc::Status ConsultarNota(grpc::ServerContext* context,
                              const gerencia_notas::AlunoDisciplinaRequest* request,
                              gerencia_notas::ConsultaNotaResponse* response) override {
      // monta a chave com RA + código da disciplina
      std::string chave = request->ra() + "_" + request->cod_disciplina();

      if (db_notas_cpp.count(chave)) {
          // se existe, retorna a nota correspondente
          const gerencia_notas::Nota& nota = db_notas_cpp[chave];
          response->mutable_nota()->CopyFrom(nota);
          response->set_sucesso(true);   // <<< USANDO SUCESSO
      } else {
          // caso não exista, sinaliza falha
          response->set_sucesso(false);  // <<< USANDO SUCESSO
          response->set_msg_erro("Nota não encontrada."); // opcional, para informar erro
      }

      return grpc::Status::OK;
  }

    grpc::Status CalcularMedia(grpc::ServerContext* context,
                               const gerencia_notas::AlunoRequest* request,
                               gerencia_notas::MediaResponse* response) override {
        std::string ra = request->ra();
        std::vector<float> notas_aluno;

        // percorre todas as notas e pega as que pertencem ao RA solicitado
        for (const auto& par : db_notas_cpp) {
            const gerencia_notas::Nota& nota = par.second;
            if (nota.ra() == ra) {
                notas_aluno.push_back(nota.nota());
            }
        }

	// se tiver notas
        if (!notas_aluno.empty()) {
            // calcula a média usando accumulate
            float soma = std::accumulate(notas_aluno.begin(), notas_aluno.end(), 0.0f);
            float media = soma / notas_aluno.size();
            response->set_sucesso(true);
            response->set_media(media);
        } else {
            response->set_sucesso(false); // caso nao tenha notas, retorna false
            response->set_media(0.0f);
        }

        return grpc::Status::OK;
    }

    // --- Implementação do Desafio ---
    grpc::Status ListarNotasAluno(grpc::ServerContext* context,
                                  const gerencia_notas::AlunoRequest* request,
                                  grpc::ServerWriter<gerencia_notas::Nota>* writer) override {
        std::string ra = request->ra();

        // percorre o banco e envia todas as notas do aluno
        for (const auto& par : db_notas_cpp) {
            const gerencia_notas::Nota& nota = par.second;
            if (nota.ra() == ra) {
                writer->Write(nota); // envia cada nota individualmente ao cliente
            }
        }

        return grpc::Status::OK;
    }
};

void RunServer() {
    std::string server_address("0.0.0.0:50052");
    GerenciadorNotasImpl service;

    grpc::ServerBuilder builder;
    builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
    builder.RegisterService(&service);

    std::unique_ptr<grpc::Server> server(builder.BuildAndStart());
    std::cout << "Servidor C++ escutando em " << server_address << std::endl;
    server->Wait();
}

int main(int argc, char** argv) {
    RunServer();
    return 0;
}
