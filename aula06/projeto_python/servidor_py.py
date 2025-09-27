import grpc
from concurrent import futures
import notas_pb2
import notas_pb2_grpc

# para adaptar pra python, usamos um dict para simular o banco de dados em memória
# a chave é "RA_CODDISCIPLINA", e o valor é um objeto Nota
db_notas_py = {}

class GerenciadorNotasServicer(notas_pb2_grpc.GerenciadorNotasServicer):

    def AdicionarNota(self, request, context):
        chave = request.ra + "_" + request.cod_disciplina
        if chave in db_notas_py:
            return notas_pb2.StatusResponse(
                sucesso=False,
                msg="Nota já existe. Use 'AlterarNota'."
            )
        else:
            nova_nota = notas_pb2.Nota(
                ra=request.ra,
                cod_disciplina=request.cod_disciplina,
                ano=request.ano,
                semestre=request.semestre,
                nota=request.nota
            )
            db_notas_py[chave] = nova_nota
            return notas_pb2.StatusResponse(
                sucesso=True,
                msg="Nota adicionada com sucesso!"
            )

    def AlterarNota(self, request, context):
        # cria a chave da mesma forma que em AdicionarNota
        chave = request.ra + "_" + request.cod_disciplina

        # verifica se a nota existe no banco
        if chave in db_notas_py:
            # atualiza os dados da nota existente
            nota = db_notas_py[chave]
            nota.ano = request.ano
            nota.semestre = request.semestre
            nota.nota = request.nota

            return notas_pb2.StatusResponse(
                sucesso=True,
                msg="Nota alterada com sucesso!"
            )
        else:
            return notas_pb2.StatusResponse(
                sucesso=False,
                msg="Nota não encontrada. Use 'AdicionarNota'."
            )

    def ConsultarNota(self, request, context):
        # monta a chave com RA + código da disciplina
        chave = request.ra + "_" + request.cod_disciplina

        if chave in db_notas_py:
            # se existe, retorna a nota correspondente
            nota = db_notas_py[chave]
            return notas_pb2.ConsultaNotaResponse(
                nota=nota,
                sucesso=True  # ALTERAÇÃO: usa sucesso como no C++
            )
        else:
            return notas_pb2.ConsultaNotaResponse(
                sucesso=False,           # ALTERAÇÃO: usa sucesso como no C++
                msg_erro="Nota não encontrada."  # ALTERAÇÃO: adiciona msg_erro como no C++
            )

    def CalcularMedia(self, request, context):
        ra = request.ra
        notas_aluno = []

        # percorre todas as notas e pega as que pertencem ao RA solicitado
        for nota in db_notas_py.values():
            if nota.ra == ra:
                notas_aluno.append(nota.nota)

        # se tiver notas
        if notas_aluno:
            # calcula a média somando e dividindo pelo total
            soma = sum(notas_aluno)
            media = soma / len(notas_aluno)
            return notas_pb2.MediaResponse(
                sucesso=True,
                media=media
            )
        else:
            return notas_pb2.MediaResponse(
                sucesso=False, # caso nao tenha notas, retorna false
                media=0.0
            )

    # --- Implementação do Desafio ---
    def ListarNotasAluno(self, request, context):
        ra = request.ra

        # percorre o banco e envia todas as notas do aluno
        for nota in db_notas_py.values():
            if nota.ra == ra:
                yield nota  # envia cada nota individualmente ao cliente


def RunServer():
    server_address = "0.0.0.0:50051"
    server = grpc.server(futures.ThreadPoolExecutor(max_workers=10))
    notas_pb2_grpc.add_GerenciadorNotasServicer_to_server(
        GerenciadorNotasServicer(), server
    )
    server.add_insecure_port(server_address)
    server.start()
    print(f"Servidor Python escutando em {server_address}")
    server.wait_for_termination()


if __name__ == "__main__":
    RunServer()