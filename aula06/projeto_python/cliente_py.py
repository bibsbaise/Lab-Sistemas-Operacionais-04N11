import grpc
import notas_pb2
import notas_pb2_grpc


class NotasClient:
    def __init__(self, channel):
        # ponteiro para o stub gerado pelo Protobuf/grpc (classe que contém os métodos RPC)
        self.stub = notas_pb2_grpc.GerenciadorNotasStub(channel)

    def testarTudo(self):
        # 1. Adicionar Notas
        print("\n1. Adicionando notas...")
        self.adicionarNota("789", "FIS0001", 2025, 1, 10.0)
        self.adicionarNota("789", "MAT0001", 2025, 1, 9.0)
        self.adicionarNota("101", "FIS0001", 2025, 1, 6.5)

        # 2. Consultar Nota
        print("\n2. Consultando nota de FIS0001 para RA 789...")
        self.consultarNota("789", "FIS0001")

        # 3. Alterar Nota
        print("\n3. Alterando nota de MAT0001 para RA 789 para 9.5...")
        self.alterarNota("789", "MAT0001", 2025, 1, 9.5)

        # 4. Calcular Média
        print("\n4. Calculando média para RA 789...")
        self.calcularMedia("789")

        # 5. Listar Notas via Streaming
        print("\n5. DESAFIO: Listando todas as notas do RA 789 via streaming...")
        self.listarNotasAluno("789")

    # Função para adicionar nota
    def adicionarNota(self, ra, cod, ano, semestre, nota_val):
        request = notas_pb2.AdicionaNotaRequest(
            ra=ra,
            cod_disciplina=cod,
            ano=ano,
            semestre=semestre,
            nota=nota_val
        )

        try:
            reply = self.stub.AdicionarNota(request)
            print(f"  - Resposta: {reply.msg}")
        except grpc.RpcError as e:
            print(f"  - Erro RPC: {e.details()}")

    # Função para alterar nota
    def alterarNota(self, ra, cod, ano, semestre, nota_val):
        # cria a mensagem de request para alteração
        request = notas_pb2.AdicionaNotaRequest(
            ra=ra,
            cod_disciplina=cod,
            ano=ano,
            semestre=semestre,
            nota=nota_val
        )

        try:
            reply = self.stub.AlterarNota(request)
            print(f"  - Resposta: {reply.msg}")
        except grpc.RpcError as e:
            print(f"  - Erro RPC: {e.details()}")

    # Função para consultar nota
    def consultarNota(self, ra, cod):
        # request para consulta (AlunoDisciplinaRequest)
        request = notas_pb2.AlunoDisciplinaRequest(
            ra=ra,
            cod_disciplina=cod
        )

        try:
            reply = self.stub.ConsultarNota(request)
            # chamada ok --> verificamos o conteúdo lógico (se a nota foi encontrada)
            if reply.sucesso:  # ALTERAÇÃO: usa sucesso como no C++
                n = reply.nota
                print("  - Nota encontrada:")
                print(f"      RA: {n.ra}")
                print(f"      Disciplina: {n.cod_disciplina}")
                print(f"      Ano: {n.ano} | Semestre: {n.semestre}")
                print(f"      Valor: {n.nota}")
            else:
                # se não foi encontrada, informa ao usuário
                print(f"  - Nota não encontrada para RA {ra} e disciplina {cod}.")
                # novo comentário: exibe a mensagem de erro do proto, se houver
                if hasattr(reply, 'msg_erro') and reply.msg_erro:
                    print(f"      Erro: {reply.msg_erro}")
        except grpc.RpcError as e:
            print(f"  - Erro RPC: {e.details()}")

    # Função para calcular média
    def calcularMedia(self, ra):
        # request do tipo AlunoRequest
        request = notas_pb2.AlunoRequest(ra=ra)

        try:
            reply = self.stub.CalcularMedia(request)
            # retorno ok --> verificamos o campo lógico 'sucesso' dentro do reply
            if reply.sucesso:
                print(f"  - Média calculada para RA {ra}: {reply.media}")
            else:
                print(f"  - Não foi possível calcular a média: nenhuma nota encontrada para RA {ra}.")
        except grpc.RpcError as e:
            print(f"  - Erro RPC: {e.details()}")

    # Função para listar notas do aluno via streaming
    def listarNotasAluno(self, ra):
        # request
        request = notas_pb2.AlunoRequest(ra=ra)

        try:
            # abre o streaming
            for nota in self.stub.ListarNotasAluno(request):
                print(f"  - Disciplina: {nota.cod_disciplina}"
                      f" | Ano: {nota.ano}"
                      f" | Semestre: {nota.semestre}"
                      f" | Nota: {nota.nota}")
            # sucesso (Python gRPC não possui reader.Finish() como C++, mas conceito equivalente)
            print("  - Listagem concluída com sucesso.")
        except grpc.RpcError as e:
            print(f"  - Erro RPC (stream): {e.details()}")


if __name__ == "__main__":
    target_str = "localhost:50052"

    import sys
    if len(sys.argv) > 1:
        target_str = sys.argv[1]

    print(f"--- Cliente Python conectando em {target_str} ---")
    with grpc.insecure_channel(target_str) as channel:
        client = NotasClient(channel)
        client.testarTudo()