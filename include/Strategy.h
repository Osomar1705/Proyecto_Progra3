#ifndef STRATEGY_H
#define STRATEGY_H

#include "Movie.h"
#include <vector>
#include <set>
#include <unordered_map>
#include <algorithm>

// ---------------------------------------------------------
// PATRON DE DISENO: STRATEGY
// ---------------------------------------------------------

class IRecommendationStrategy {
public:
    virtual std::vector<int> recommend(const std::set<int>& likedMovies,
                                       const std::vector<Movie>& allMovies,
                                       const std::unordered_map<int, int>& movieIndexMap) = 0;
    virtual ~IRecommendationStrategy() = default;
};

class GenreKeywordRecommendation : public IRecommendationStrategy {
public:
    std::vector<int> recommend(const std::set<int>& likedMovies,
                               const std::vector<Movie>& allMovies,
                               const std::unordered_map<int, int>& movieIndexMap) override {
        if (likedMovies.empty()) return {};

        std::unordered_map<std::string, int> genreCount;
        std::unordered_map<std::string, int> keywordCount;

        for (int mid : likedMovies) {
            auto it = movieIndexMap.find(mid);
            if (it != movieIndexMap.end()) {
                const Movie& m = allMovies[it->second];
                for (const std::string& g : m.clean_genre) genreCount[g]++;
                for (const std::string& t : m.clean_title) keywordCount[t]++;
            }
        }

        std::unordered_map<int, int> scores;
        for (const auto& m : allMovies) {
            if (likedMovies.count(m.id)) continue;

            int score = 0;
            for (const std::string& g : m.clean_genre) {
                auto g_it = genreCount.find(g);
                if (g_it != genreCount.end()) score += g_it->second * 2;
            }
            for (const std::string& t : m.clean_title) {
                auto k_it = keywordCount.find(t);
                if (k_it != keywordCount.end()) score += k_it->second;
            }
            if (score > 0) scores[m.id] = score;
        }

        std::vector<std::pair<int, int>> ranked;
        for (auto const& [id, score] : scores) ranked.push_back({score, id});
        std::sort(ranked.rbegin(), ranked.rend());

        std::vector<int> result;
        for (int i = 0; i < std::min((int)ranked.size(), 5); ++i) {
            result.push_back(ranked[i].second);
        }
        return result;
    }
};

#endif // STRATEGY_H
