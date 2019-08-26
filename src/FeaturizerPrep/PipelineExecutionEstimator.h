// ----------------------------------------------------------------------
// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License
// ----------------------------------------------------------------------
#pragma once

#include "Featurizer.h"
#include "Details/PipelineExecutionEstimator_details.h"

namespace Microsoft {
namespace Featurizer {

/////////////////////////////////////////////////////////////////////////
///  \class         PipelineExecutionEstimator
///  \brief         Executes one or more `Estimators` in sequential
///                 order. This object is used to stich together individual
///                 `Estimators` into a larger directed acyclic graph (DAG).
///
template <typename... EstimatorT>
class PipelineExecutionEstimator : public TransformerEstimator<
    typename Details::PipelineTraits<EstimatorT...>::InputType,
    typename Details::PipelineTraits<EstimatorT...>::TransformedType
> {
public:
    // ----------------------------------------------------------------------
    // |
    // |  Public Types
    // |
    // ----------------------------------------------------------------------
    using PipelineTraits                    = Details::PipelineTraits<EstimatorT...>;

    using ThisType                          = PipelineExecutionEstimator<EstimatorT...>;
    using BaseType                          = TransformerEstimator<typename PipelineTraits::InputType, typename PipelineTraits::TransformedType>;

    using InputType                         = typename BaseType::InputType;
    using FitBufferInputType                = typename BaseType::FitBufferInputType;
    using FitResult                         = typename BaseType::FitResult;
    using TransformerUniquePtr              = typename BaseType::TransformerUniquePtr;
    using TransformedType                   = typename BaseType::TransformedType;

    // ----------------------------------------------------------------------
    // |
    // |  Public Methods
    // |
    // ----------------------------------------------------------------------
    PipelineExecutionEstimator(std::string name, AnnotationMapsPtr pAllColumnAnnotations);

private:
    // ----------------------------------------------------------------------
    // |
    // |  Private Types
    // |
    // ----------------------------------------------------------------------
    using EstimatorChain                    = Details::EstimatorChain<typename PipelineTraits::Pipeline>;

    class Transformer : public BaseType::Transformer {
    public:
        // ----------------------------------------------------------------------
        // |  Public Types
        using EstimatorChain                = EstimatorChain;

        // ----------------------------------------------------------------------
        // |  Public Methods
        Transformer(EstimatorChain &estimator_chain);
        Transformer(Archive &ar);

        ~Transformer(void) override = default;

        FEATURIZER_MOVE_CONSTRUCTOR_ONLY(Transformer);

        TransformedType execute(InputType input) override;
        void save(Archive &ar) const override;

    private:
        // ----------------------------------------------------------------------
        // |  Private Types
        using TransformerChain              = Details::TransformerChain<typename PipelineTraits::Pipeline>;

        // ----------------------------------------------------------------------
        // |  Private Data
        TransformerChain                    _transformer_chain;
    };

    // ----------------------------------------------------------------------
    // |
    // |  Private Data
    // |
    // ----------------------------------------------------------------------
    EstimatorChain                          _estimator_chain;

    // ----------------------------------------------------------------------
    // |
    // |  Private Methods
    // |
    // ----------------------------------------------------------------------
    FitResult fit_impl(FitBufferInputType const *pBuffer, size_t cBuffer) override;
    FitResult complete_training_impl(void) override;
    TransformerUniquePtr create_transformer_impl(void) override;
};

// ----------------------------------------------------------------------
// ----------------------------------------------------------------------
// ----------------------------------------------------------------------
// |
// |  Implementation
// |
// ----------------------------------------------------------------------
// ----------------------------------------------------------------------
// ----------------------------------------------------------------------

// ----------------------------------------------------------------------
// |
// |  PipelineExecutionEstimator
// |
// ----------------------------------------------------------------------
template <typename... EstimatorT>
PipelineExecutionEstimator<EstimatorT...>::PipelineExecutionEstimator(std::string name, AnnotationMapsPtr pAllColumnAnnotations) :
    BaseType(std::move(name), pAllColumnAnnotations),
    _estimator_chain(pAllColumnAnnotations) {
        if(_estimator_chain.is_all_training_complete())
            this->complete_training();
}

template <typename... EstimatorT>
typename PipelineExecutionEstimator<EstimatorT...>::FitResult PipelineExecutionEstimator<EstimatorT...>::fit_impl(FitBufferInputType const *pBuffer, size_t cBuffer) /*override*/ {
    return _estimator_chain.fit(pBuffer, cBuffer);
}

template <typename... EstimatorT>
typename PipelineExecutionEstimator<EstimatorT...>::FitResult PipelineExecutionEstimator<EstimatorT...>::complete_training_impl(void) /*override*/ {
    return _estimator_chain.complete_training(true);
}

template <typename... EstimatorT>
typename PipelineExecutionEstimator<EstimatorT...>::TransformerUniquePtr PipelineExecutionEstimator<EstimatorT...>::create_transformer_impl(void) /*override*/ {
    return std::make_unique<Transformer>(_estimator_chain);
}

// ----------------------------------------------------------------------
// |
// |  PipelineExecutionEstimator::Transformer
// |
// ----------------------------------------------------------------------
template <typename... EstimatorT>
PipelineExecutionEstimator<EstimatorT...>::Transformer::Transformer(EstimatorChain &estimator_chain) :
    _transformer_chain(estimator_chain) {
}

template <typename... EstimatorT>
PipelineExecutionEstimator<EstimatorT...>::Transformer::Transformer(Archive &ar) :
    _transformer_chain(ar) {
}

template <typename... EstimatorT>
typename PipelineExecutionEstimator<EstimatorT...>::TransformedType PipelineExecutionEstimator<EstimatorT...>::Transformer::execute(InputType input) /*override*/ {
    return _transformer_chain.execute(input);
}

template <typename... EstimatorT>
void PipelineExecutionEstimator<EstimatorT...>::Transformer::save(Archive &ar) const /*override*/ {
    _transformer_chain.save(ar);
}

} // namespace Featurizer
} // namespace Microsoft
