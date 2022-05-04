#include "Veteran.h"

bool Veteran::result_critical_section(bool won)
{
	//todo:
	return true;
}

void Veteran::result_end_duel(bool won)
{
	if (!won)
		if (!result_critical_section(won))
			return;
	{
		const std::lock_guard<std::mutex> lock(m_busy);
		b_busy = false;
	}
	send_vet_readiness(t_id);
	m_challenge.unlock();
}

void Veteran::result()
{
	MPI_Status status;
	char data;
	while (true)
	{
		MPI_Recv(&data, 1, MPI_CHAR, MPI_ANY_SOURCE, Message::vet_duel_results, MPI_COMM_WORLD, &status);
		
		//cancelled duel
		if (data == 2)
		{
			{
				const std::lock_guard<std::mutex> lock(m_busy);
				b_busy = false;
			}
			m_challenge.unlock();
		}
		else result_end_duel(data);
	}
}

void Veteran::free_bed()
{
	while (true)
	{
		MPI_Recv(nullptr, 0, MPI_DATATYPE_NULL, MPI_ANY_SOURCE, Message::vet_bed_freed, MPI_COMM_WORLD, nullptr);
		{
			const std::lock_guard<std::mutex> lock(m_wounded);
			if (b_wounded)
				result_end_duel(false);
		}
	}
}
