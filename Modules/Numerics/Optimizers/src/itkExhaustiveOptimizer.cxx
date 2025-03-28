/*=========================================================================
 *
 *  Copyright NumFOCUS
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *         https://www.apache.org/licenses/LICENSE-2.0.txt
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 *
 *=========================================================================*/
#include "itkExhaustiveOptimizer.h"

namespace itk
{

ExhaustiveOptimizer::ExhaustiveOptimizer() = default;

void
ExhaustiveOptimizer::StartOptimization()
{
  this->StartWalking();
}

void
ExhaustiveOptimizer::StartWalking()
{
  itkDebugMacro("StartWalking");
  this->InvokeEvent(StartEvent());
  m_StopConditionDescription.str("");
  m_StopConditionDescription << this->GetNameOfClass() << ": Running";

  const ParametersType initialPos = this->GetInitialPosition();
  m_MinimumMetricValuePosition = initialPos;
  m_MaximumMetricValuePosition = initialPos;

  const MeasureType initialValue = this->GetValue(this->GetInitialPosition());
  m_MaximumMetricValue = initialValue;
  m_MinimumMetricValue = initialValue;

  m_CurrentIteration = 0;
  m_MaximumNumberOfIterations = 1;

  const unsigned int spaceDimension = this->GetInitialPosition().GetSize();

  for (unsigned int i = 0; i < spaceDimension; ++i)
  {
    m_MaximumNumberOfIterations *= (2 * m_NumberOfSteps[i] + 1);
  }

  m_CurrentIndex.SetSize(spaceDimension);
  m_CurrentIndex.Fill(0);

  const ScalesType & scales = this->GetScales();
  // Make sure the scales have been set properly
  if (scales.size() != spaceDimension)
  {
    itkExceptionMacro("The size of Scales is " << scales.size() << ", but the NumberOfParameters is " << spaceDimension
                                               << '.');
  }

  // Setup first grid position.
  ParametersType position(spaceDimension);
  for (unsigned int i = 0; i < spaceDimension; ++i)
  {
    position[i] = this->GetInitialPosition()[i] - m_NumberOfSteps[i] * m_StepLength * scales[i];
  }
  this->SetCurrentPosition(position);

  itkDebugMacro("Calling ResumeWalking");

  this->ResumeWalking();
}

void
ExhaustiveOptimizer::ResumeWalking()
{
  itkDebugMacro("ResumeWalk");
  m_Stop = false;

  while (!m_Stop)
  {
    const ParametersType currentPosition = this->GetCurrentPosition();

    if (m_Stop)
    {
      StopWalking();
      break;
    }

    m_CurrentValue = this->GetValue(currentPosition);

    if (m_CurrentValue > m_MaximumMetricValue)
    {
      m_MaximumMetricValue = m_CurrentValue;
      m_MaximumMetricValuePosition = currentPosition;
    }
    if (m_CurrentValue < m_MinimumMetricValue)
    {
      m_MinimumMetricValue = m_CurrentValue;
      m_MinimumMetricValuePosition = currentPosition;
    }

    if (m_Stop)
    {
      this->StopWalking();
      break;
    }

    m_StopConditionDescription.str("");
    m_StopConditionDescription << this->GetNameOfClass() << ": Running. "
                               << "@ index " << this->GetCurrentIndex() << " value is " << this->GetCurrentValue();

    this->InvokeEvent(IterationEvent());
    this->AdvanceOneStep();
    ++m_CurrentIteration;
  }
}

void
ExhaustiveOptimizer::StopWalking()
{
  itkDebugMacro("StopWalking");

  m_Stop = true;
  this->InvokeEvent(EndEvent());
}

void
ExhaustiveOptimizer::AdvanceOneStep()
{
  itkDebugMacro("AdvanceOneStep");

  const unsigned int spaceDimension = m_CostFunction->GetNumberOfParameters();

  ParametersType newPosition(spaceDimension);
  IncrementIndex(newPosition);

  itkDebugMacro("new position = " << newPosition);

  this->SetCurrentPosition(newPosition);
}

void
ExhaustiveOptimizer::IncrementIndex(ParametersType & newPosition)
{
  unsigned int       idx = 0;
  const unsigned int spaceDimension = m_CostFunction->GetNumberOfParameters();

  while (idx < spaceDimension)
  {
    m_CurrentIndex[idx]++;

    if (m_CurrentIndex[idx] > (2 * m_NumberOfSteps[idx]))
    {
      m_CurrentIndex[idx] = 0;
      ++idx;
    }
    else
    {
      break;
    }
  }

  if (idx == spaceDimension)
  {
    m_Stop = true;
    m_StopConditionDescription.str("");
    m_StopConditionDescription << this->GetNameOfClass() << ": "
                               << "Completed sampling of parametric space of size " << spaceDimension;
  }

  const ScalesType & scales = this->GetScales();
  for (unsigned int i = 0; i < spaceDimension; ++i)
  {
    newPosition[i] =
      (m_CurrentIndex[i] - m_NumberOfSteps[i]) * m_StepLength * scales[i] + this->GetInitialPosition()[i];
  }
}

std::string
ExhaustiveOptimizer::GetStopConditionDescription() const
{
  return m_StopConditionDescription.str();
}

void
ExhaustiveOptimizer::PrintSelf(std::ostream & os, Indent indent) const
{
  Superclass::PrintSelf(os, indent);

  os << indent << "CurrentValue: " << static_cast<typename NumericTraits<MeasureType>::PrintType>(m_CurrentValue)
     << std::endl;
  os << indent << "NumberOfSteps: " << static_cast<typename NumericTraits<StepsType>::PrintType>(m_NumberOfSteps)
     << std::endl;
  os << indent
     << "CurrentIteration: " << static_cast<typename NumericTraits<SizeValueType>::PrintType>(m_CurrentIteration)
     << std::endl;
  itkPrintSelfBooleanMacro(Stop);
  os << indent << "CurrentParameter: " << m_CurrentParameter << std::endl;
  os << indent << "StepLength: " << m_StepLength << std::endl;
  os << indent << "CurrentIndex: " << m_CurrentIndex << std::endl;
  os << indent << "MaximumNumberOfIterations: "
     << static_cast<typename NumericTraits<SizeValueType>::PrintType>(m_MaximumNumberOfIterations) << std::endl;
  os << indent
     << "MaximumMetricValue: " << static_cast<typename NumericTraits<MeasureType>::PrintType>(m_MaximumMetricValue)
     << std::endl;
  os << indent
     << "MinimumMetricValue: " << static_cast<typename NumericTraits<MeasureType>::PrintType>(m_MinimumMetricValue)
     << std::endl;
  os << indent << "MinimumMetricValuePosition: " << m_MinimumMetricValuePosition << std::endl;
  os << indent << "MaximumMetricValuePosition: " << m_MaximumMetricValuePosition << std::endl;

  os << indent << "StopConditionDescription: " << m_StopConditionDescription.str() << std::endl;
}
} // end namespace itk
