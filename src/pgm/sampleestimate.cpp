#include <pgm/sampleestimate.h>
#include <pgm/bayesnet.h>
#include <pgm/dataset.h>
#include <pgm/frequency.h>

using namespace pgm;

SampleEstimate::SampleEstimate(double alpha)
    : alpha_(alpha)
{}

void SampleEstimate::operator ()(Bayesnet &bayesnet, const Dataset &dataset)
{
    for (auto vertex : bayesnet.graph().vertices())
    {
        auto adjacents = bayesnet.graph().adjacents(vertex);

        Variable var_child = bayesnet.variable(vertex);
        std::string child = var_child.name();
        std::vector<std::string> parents;
        for (auto adj_id : adjacents)
            parents.push_back(bayesnet.variable(adj_id).name());

        auto parents_child = parents;
        parents_child.push_back(child);

        Frequency count(dataset, parents_child);
        auto pa_i = count.permutate(parents);

        double alpha_ijk = alpha_ / double(pa_i.size() * var_child.cardinality());
        for (std::size_t j = 0; j < pa_i.size(); ++j)
        {
            std::unordered_map<std::string, std::string> vars;
            for (std::size_t p = 0; p < pa_i[j].size(); ++p)
                vars[parents[p]] = pa_i[j][p];

            double denom = pa_i.size() * alpha_ijk + count(vars);

            for (std::size_t k = 0; k < var_child.cardinality(); ++k)
            {
                std::string child_state = var_child(k);
                vars[child] = child_state;
                double p = (alpha_ijk + count(vars)) / denom;
                bayesnet.probability(child, child_state, vars, p);
            }
        }
    }
}
