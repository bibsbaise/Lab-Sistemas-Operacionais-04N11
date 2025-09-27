#include <iostream>
#include <memory>
#include <string>

#include <grpcpp/grpcpp.h>
#include "notas.grpc.pb.h"

class NotasClient {
public:
    NotasClient(std::shared_ptr<grpc::Channel> channel)
        : stub_(gerencia_notas::GerenciadorNotas::NewStub(channel)) {}

    void testarTudo() {
        // 1. Adicionar Notas
        std::cout << "\n1. Adicionando notas..." << std::endl;
        adicionarNota("789", "FIS0001", 2025, 1, 10.0);
        adicionarNota("789", "MAT0001", 2025, 1, 9.0);
        adicionarNota("101", "FIS0001", 2025, 1, 6.5);

        // 2. Consultar Nota
        std::cout << "\n2. Consultando nota de FIS0001 para RA 789..." << std::endl;
        consultarNota("789", "FIS0001");

        // 3. Alterar Nota
        std::cout << "\n3. Alterando nota de MAT0001 para RA 789 para 9.5..." << std::endl;
        alterarNota("789", "MAT0001", 2025, 1, 9.5);

        // 4. Calcular Média
        std::cout << "\n4. Calculando média para RA 789..." << std::endl;
        calcularMedia("789");

        // 5. Listar Notas via Streaming
        std::cout << "\n5. DESAFIO: Listando todas as notas do RA 789 via streaming..." << std::endl;
        listarNotasAluno("789");
    }

private:
    // Função para adicionar nota
    void adicionarNota(const std::string& ra, const std::string& cod, int ano, int semestre, float nota_val) {

        gerencia_notas::AdicionaNotaRequest request;
        request.set_ra(ra);
        request.set_cod_disciplina(cod);
        request.set_ano(ano);
        request.set_semestre(semestre);
        request.set_nota(nota_val);

        gerencia_notas::StatusResponse reply;
        grpc::ClientContext context;

        grpc::Status status = stub_->AdicionarNota(&context, request, &reply);

        if (status.ok()) {
            std::cout << "  - Resposta: " << reply.msg() << std::endl;
        } else {
            std::cout << "  - Erro RPC: " << status.error_message() << std::endl;
        }
    }

    // Função para alterar nota
    void alterarNota(const std::string& ra, const std::string& cod, int ano, int semestre, float nota_val) {

        // cria a mensagem de request para alteração
        gerencia_notas::AdicionaNotaRequest request;

        request.set_ra(ra);
        request.set_cod_disciplina(cod);
        request.set_ano(ano);
        request.set_semestre(semestre);
        request.set_nota(nota_val);

        // prepara a resposta e contexto
        gerencia_notas::StatusResponse reply;
        grpc::ClientContext context;

        // chama o RPC AlterarNota no stub
        grpc::Status status = stub_->AlterarNota(&context, request, &reply);

        // trata o retorno
        if (status.ok()) {
            std::cout << "  - Resposta: " << reply.msg() << std::endl;
        } else {
            std::cout << "  - Erro RPC: " << status.error_message() << std::endl;
        }
    }

    // Função para consultar nota
    void consultarNota(const std::string& ra, const std::string& cod) {

        // request  para consulta (AlunoDisciplinaRequest)
        gerencia_notas::AlunoDisciplinaRequest request;

        request.set_ra(ra);
        request.set_cod_disciplina(cod);

        // o servidor usa ConsultaNotaResponse, que contém:
        //  - um campo booleano "sucesso" (se a operação foi bem-sucedida)
        //  - um campo "nota" (mensagem Nota) quando sucesso=true
        gerencia_notas::ConsultaNotaResponse reply;
        grpc::ClientContext context;

        // chamada RPC síncrona para ConsultarNota
        grpc::Status status = stub_->ConsultarNota(&context, request, &reply);

        // trata retorno
        if (status.ok()) {
            // chamada ok --> verificamos o conteúdo lógico (se a nota foi encontrada)
            // ALTERAÇÃO: usa 'sucesso' do proto em vez de 'encontrada'
            if (reply.sucesso()) {
                // se foi encontrada, reply.nota() é um objeto gerencia_notas::Nota
                // mostramos os detalhes: disciplina, ano, semestre e valor da nota
                const gerencia_notas::Nota& n = reply.nota();
                std::cout << "  - Nota encontrada:" << std::endl;
                std::cout << "      RA: " << n.ra() << std::endl;
                std::cout << "      Disciplina: " << n.cod_disciplina() << std::endl;
                std::cout << "      Ano: " << n.ano() << " | Semestre: " << n.semestre() << std::endl;
                std::cout << "      Valor: " << n.nota() << std::endl;
            } else {
                // se não foi encontrada, informa ao usuário
                std::cout << "  - Nota não encontrada para RA " << ra << " e disciplina " << cod << "." << std::endl;
                // novo comentário: exibe a mensagem de erro do proto, se houver
                if (!reply.msg_erro().empty()) {
                    std::cout << "      Erro: " << reply.msg_erro() << std::endl;
                }
            }
        } else {
            std::cout << "  - Erro RPC: " << status.error_message() << std::endl;
        }
    }

    // Função para calcular média
    void calcularMedia(const std::string& ra) {

        // request do tipo AlunoRequest
        gerencia_notas::AlunoRequest request;

        request.set_ra(ra);

        // MediaResponse deverá conter: sucesso (bool) e media (float)
        gerencia_notas::MediaResponse reply;
        grpc::ClientContext context;

        // chama o RPC CalcularMedia
        grpc::Status status = stub_->CalcularMedia(&context, request, &reply);

        // trata retorno
        if (status.ok()) {
            // retorno ok --> verificamos o campo lógico 'sucesso' dentro do reply
            if (reply.sucesso()) {
                // mostra a média calculada pelo servidor
                std::cout << "  - Média calculada para RA " << ra << ": " << reply.media() << std::endl;
            } else {
                // se não há notas, o servidor pode retornar sucesso=false
                std::cout << "  - Não foi possível calcular a média: nenhuma nota encontrada para RA " << ra << "." << std::endl;
            }
        } else {
            std::cout << "  - Erro RPC: " << status.error_message() << std::endl;
        }
    }

    // Função para listar notas do aluno via streaming
    void listarNotasAluno(const std::string& ra) {

        // request
        gerencia_notas::AlunoRequest request;

        request.set_ra(ra);

        // cria o contexto do cliente para a chamada streaming
        grpc::ClientContext context;

        // abre um reader para receber mensagens do servidor
        std::unique_ptr<grpc::ClientReader<gerencia_notas::Nota>> reader(
            stub_->ListarNotasAluno(&context, request));

        // objeto que será preenchido a cada leitura do stream
        gerencia_notas::Nota nota;

        // reader->Read(blocking) retorna true enquanto houver mensagens para ler.
        while (reader->Read(&nota)) {
            // p/ cada nota recebida, exibimos os campos necessários
            std::cout << "  - Disciplina: " << nota.cod_disciplina()
                      << " | Ano: " << nota.ano()
                      << " | Semestre: " << nota.semestre()
                      << " | Nota: " << nota.nota() << std::endl;
        }

        // finaliza o reader e checa o status final da stream
        grpc::Status status = reader->Finish();
        if (!status.ok()) {
            // erro
            std::cout << "  - Erro RPC (stream): " << status.error_message() << std::endl;
        } else {
            // sucesso
            std::cout << "  - Listagem concluída com sucesso." << std::endl;
        }
    }

    // ponteiro para o stub gerado pelo Protobuf/grpc (classe que contém os métodos RPC)
    std::unique_ptr<gerencia_notas::GerenciadorNotas::Stub> stub_;
};

int main(int argc, char** argv) {
    std::string target_str = "localhost:50051";

    if (argc > 1) {
        target_str = argv[1];
    }

    std::cout << "--- Cliente C++ conectando em " << target_str << " ---" << std::endl;
    NotasClient client(grpc::CreateChannel(target_str, grpc::InsecureChannelCredentials()));
    client.testarTudo();

    return 0;
}