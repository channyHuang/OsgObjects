#ifndef POISSONRECON_H
#define POISSONRECON_H

#include <cstring>
#include <string>

class PoissonRecon2020 {
public:
	static PoissonRecon2020* getInstance() {
		if (instance == nullptr) {
			instance = new PoissonRecon2020();
		}
		return instance;
	}

	int Poisson(std::string& inFileName, std::string outFileName = "recon_res.ply");

private:
	PoissonRecon2020() {}
	~PoissonRecon2020() {}

	static PoissonRecon2020* instance;
};

#endif