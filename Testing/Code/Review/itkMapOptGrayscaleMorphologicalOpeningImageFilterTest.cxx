/*=========================================================================

  Program:   Insight Segmentation & Registration Toolkit
  Module:    itkMapOptGrayscaleMorphologicalOpeningImageFilterTest.cxx
  Language:  C++
  Date:      $Date$
  Version:   $Revision$

  Copyright (c) Insight Software Consortium. All rights reserved.
  See ITKCopyright.txt or http://www.itk.org/HTML/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#if defined(_MSC_VER)
#pragma warning ( disable : 4786 )
#endif

#include <fstream>
#include "itkOptGrayscaleMorphologicalOpeningImageFilter.h"
#include "itkImageFileReader.h"
#include "itkImageFileWriter.h"
#include "itkTextOutput.h"
#include "itkNumericTraits.h"
#include "itkFilterWatcher.h"
#include "itkSimpleFilterWatcher.h"

int itkMapOptGrayscaleMorphologicalOpeningImageFilterTest(int ac, char* av[] )
{
  // Comment the following if you want to use the itk text output window
  itk::OutputWindow::SetInstance(itk::TextOutput::New());

  if(ac < 7)
    {
    std::cerr << "Usage: " << av[0] << " InputImage BASIC HISTO ANCHOR VHGW SafeBorder" << std::endl;
    return -1;
    }

  unsigned int const dim = 2;
  typedef itk::Image<unsigned short, dim> ImageType;
  
  typedef itk::ImageFileReader<ImageType> ReaderType;
  ReaderType::Pointer reader  = ReaderType::New();
  reader->SetFileName(av[1]);
  
  // Create a filter
  typedef itk::FlatStructuringElement<dim> SRType;
  typedef itk::GrayscaleMorphologicalOpeningImageFilter< ImageType, ImageType, SRType > FilterType;
  FilterType::Pointer filter = FilterType::New();
  filter->SetInput( reader->GetOutput() );

  itk::SimpleFilterWatcher watcher(filter, "filter");

  typedef FilterType::RadiusType RadiusType;

  // test default values
  RadiusType r1;
  r1.Fill( 1 );
  if ( filter->GetRadius() != r1 )
    {
    std::cerr << "Wrong default Radius: " << filter->GetRadius() << std::endl;
    return EXIT_FAILURE;
    }

  if ( filter->GetAlgorithm() != FilterType::HISTO )
    {
    std::cerr << "Wrong default algorithm." << std::endl;
    return EXIT_FAILURE;
    }

  if ( filter->GetSafeBorder() != false )
    {
    std::cerr << "Wrong default safe border." << std::endl;
    return EXIT_FAILURE;
    }

  try
    {
    filter->SetRadius( 20 );
    filter->SetSafeBorder( atoi(av[6]) );

    typedef itk::ImageFileWriter< ImageType > WriterType;
    WriterType::Pointer writer = WriterType::New();
    writer->SetInput( filter->GetOutput() );
  
    filter->SetAlgorithm( FilterType::BASIC );
    writer->SetFileName( av[2] );
    writer->Update();
  
    filter->SetAlgorithm( FilterType::HISTO );
    writer->SetFileName( av[3] );
    writer->Update();
  
    filter->SetAlgorithm( FilterType::ANCHOR );
    writer->SetFileName( av[4] );
    writer->Update();
  
    filter->SetAlgorithm( FilterType::VHGW );
    writer->SetFileName( av[5] );
    writer->Update();
    }
  catch (itk::ExceptionObject& e)
    {
    std::cerr << "Exception detected: "  << e.GetDescription();
    return EXIT_FAILURE;
    }

  // Generate test image
  typedef itk::ImageFileWriter<ImageType> WriterType;
  WriterType::Pointer writer = WriterType::New();
  writer->SetInput( filter->GetOutput() );
  writer->SetFileName( av[2] );
  writer->Update();

  return EXIT_SUCCESS;
}
